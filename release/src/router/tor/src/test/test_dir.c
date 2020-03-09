/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"
#include <math.h>

#define BWAUTH_PRIVATE
#define CONFIG_PRIVATE
#define CONTROL_GETINFO_PRIVATE
#define DIRCACHE_PRIVATE
#define DIRCLIENT_PRIVATE
#define DIRSERV_PRIVATE
#define DIRVOTE_PRIVATE
#define DLSTATUS_PRIVATE
#define HIBERNATE_PRIVATE
#define NETWORKSTATUS_PRIVATE
#define NS_PARSE_PRIVATE
#define NODE_SELECT_PRIVATE
#define RELAY_PRIVATE
#define ROUTERLIST_PRIVATE
#define ROUTER_PRIVATE
#define UNPARSEABLE_PRIVATE
#define VOTEFLAGS_PRIVATE

#include "core/or/or.h"
#include "app/config/config.h"
#include "lib/confmgt/confparse.h"
#include "core/mainloop/connection.h"
#include "core/or/relay.h"
#include "core/or/versions.h"
#include "feature/client/bridges.h"
#include "feature/client/entrynodes.h"
#include "feature/control/control_getinfo.h"
#include "feature/dirauth/bwauth.h"
#include "feature/dirauth/dirvote.h"
#include "feature/dirauth/dsigs_parse.h"
#include "feature/dirauth/process_descs.h"
#include "feature/dirauth/recommend_pkg.h"
#include "feature/dirauth/shared_random_state.h"
#include "feature/dirauth/voteflags.h"
#include "feature/dircache/dircache.h"
#include "feature/dircache/dirserv.h"
#include "feature/dirclient/dirclient.h"
#include "feature/dirclient/dlstatus.h"
#include "feature/dircommon/directory.h"
#include "feature/dircommon/fp_pair.h"
#include "feature/dircommon/voting_schedule.h"
#include "feature/hibernate/hibernate.h"
#include "feature/nodelist/authcert.h"
#include "feature/nodelist/dirlist.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/nickname.h"
#include "feature/nodelist/node_select.h"
#include "feature/nodelist/routerlist.h"
#include "feature/dirparse/authcert_parse.h"
#include "feature/dirparse/ns_parse.h"
#include "feature/dirparse/routerparse.h"
#include "feature/dirparse/unparseable.h"
#include "feature/nodelist/routerset.h"
#include "feature/nodelist/torcert.h"
#include "feature/relay/router.h"
#include "feature/relay/routerkeys.h"
#include "feature/relay/routermode.h"
#include "lib/compress/compress.h"
#include "lib/crypt_ops/crypto_ed25519.h"
#include "lib/crypt_ops/crypto_format.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/encoding/confline.h"
#include "lib/memarea/memarea.h"
#include "lib/osinfo/uname.h"
#include "test/log_test_helpers.h"
#include "test/test.h"
#include "test/test_dir_common.h"

#include "core/or/addr_policy_st.h"
#include "feature/nodelist/authority_cert_st.h"
#include "feature/nodelist/document_signature_st.h"
#include "feature/nodelist/extrainfo_st.h"
#include "feature/nodelist/networkstatus_st.h"
#include "feature/nodelist/networkstatus_voter_info_st.h"
#include "feature/dirauth/ns_detached_signatures_st.h"
#include "core/or/port_cfg_st.h"
#include "feature/nodelist/routerinfo_st.h"
#include "feature/nodelist/routerlist_st.h"
#include "core/or/tor_version_st.h"
#include "feature/dirauth/vote_microdesc_hash_st.h"
#include "feature/nodelist/vote_routerstatus_st.h"

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#define NS_MODULE dir

static networkstatus_t *
networkstatus_parse_vote_from_string_(const char *s,
                                      const char **eos_out,
                                      enum networkstatus_type_t ns_type)
{
  size_t len = strlen(s);
  // memdup so that it won't be nul-terminated.
  char *tmp = tor_memdup(s, len);
  networkstatus_t *result =
    networkstatus_parse_vote_from_string(tmp, len, eos_out, ns_type);
  if (eos_out && *eos_out) {
    *eos_out = s + (*eos_out - tmp);
  }
  tor_free(tmp);
  return result;
}

static void
test_dir_nicknames(void *arg)
{
  (void)arg;
  tt_assert( is_legal_nickname("a"));
  tt_assert(!is_legal_nickname(""));
  tt_assert(!is_legal_nickname("abcdefghijklmnopqrst")); /* 20 chars */
  tt_assert(!is_legal_nickname("hyphen-")); /* bad char */
  tt_assert( is_legal_nickname("abcdefghijklmnopqrs")); /* 19 chars */
  tt_assert(!is_legal_nickname("$AAAAAAAA01234AAAAAAAAAAAAAAAAAAAAAAAAAAA"));
  /* valid */
  tt_assert( is_legal_nickname_or_hexdigest(
                                 "$AAAAAAAA01234AAAAAAAAAAAAAAAAAAAAAAAAAAA"));
  tt_assert( is_legal_nickname_or_hexdigest(
                         "$AAAAAAAA01234AAAAAAAAAAAAAAAAAAAAAAAAAAA=fred"));
  tt_assert( is_legal_nickname_or_hexdigest(
                         "$AAAAAAAA01234AAAAAAAAAAAAAAAAAAAAAAAAAAA~fred"));
  /* too short */
  tt_assert(!is_legal_nickname_or_hexdigest(
                                 "$AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"));
  /* illegal char */
  tt_assert(!is_legal_nickname_or_hexdigest(
                                 "$AAAAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"));
  /* hex part too long */
  tt_assert(!is_legal_nickname_or_hexdigest(
                         "$AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"));
  tt_assert(!is_legal_nickname_or_hexdigest(
                         "$AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=fred"));
  /* Bad nickname */
  tt_assert(!is_legal_nickname_or_hexdigest(
                         "$AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA="));
  tt_assert(!is_legal_nickname_or_hexdigest(
                         "$AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA~"));
  tt_assert(!is_legal_nickname_or_hexdigest(
                       "$AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA~hyphen-"));
  tt_assert(!is_legal_nickname_or_hexdigest(
                       "$AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA~"
                       "abcdefghijklmnoppqrst"));
  /* Bad extra char. */
  tt_assert(!is_legal_nickname_or_hexdigest(
                         "$AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA!"));
  tt_assert(is_legal_nickname_or_hexdigest("xyzzy"));
  tt_assert(is_legal_nickname_or_hexdigest("abcdefghijklmnopqrs"));
  tt_assert(!is_legal_nickname_or_hexdigest("abcdefghijklmnopqrst"));
 done:
  ;
}

/* Allocate and return a new routerinfo, with the fields set from the
 * arguments to this function.
 *
 * Also sets:
 *  - random RSA identity and onion keys,
 *  - the platform field using get_platform_str(), and
 *  - supports_tunnelled_dir_requests to 1.
 *
 * If rsa_onion_keypair_out is not NULL, it is set to the onion keypair.
 * The caller must free this keypair.
 */
static routerinfo_t *
basic_routerinfo_new(const char *nickname, uint32_t ipv4_addr,
                     uint16_t or_port, uint16_t dir_port,
                     uint32_t bandwidthrate, uint32_t bandwidthburst,
                     uint32_t bandwidthcapacity,
                     time_t published_on,
                     crypto_pk_t **rsa_onion_keypair_out)
{
  char platform[256];

  tor_assert(nickname);

  crypto_pk_t *pk1 = NULL, *pk2 = NULL;
  /* These keys are random: idx is ignored. */
  pk1 = pk_generate(0);
  pk2 = pk_generate(1);

  tor_assert(pk1);
  tor_assert(pk2);

  get_platform_str(platform, sizeof(platform));

  routerinfo_t *r1 = tor_malloc_zero(sizeof(routerinfo_t));

  r1->nickname = tor_strdup(nickname);
  r1->platform = tor_strdup(platform);

  r1->addr = ipv4_addr;
  r1->or_port = or_port;
  r1->dir_port = dir_port;
  r1->supports_tunnelled_dir_requests = 1;

  router_set_rsa_onion_pkey(pk1, &r1->onion_pkey, &r1->onion_pkey_len);
  r1->identity_pkey = pk2;

  r1->bandwidthrate = bandwidthrate;
  r1->bandwidthburst = bandwidthburst;
  r1->bandwidthcapacity = bandwidthcapacity;

  r1->cache_info.published_on = published_on;

  if (rsa_onion_keypair_out) {
    *rsa_onion_keypair_out = pk1;
  } else {
    crypto_pk_free(pk1);
  }

  return r1;
}

/* Allocate and return a new string containing a "router" line for r1. */
static char *
get_new_router_line(const routerinfo_t *r1)
{
  char *line = NULL;

  tor_assert(r1);

  tor_asprintf(&line,
               "router %s %s %d 0 %d\n",
               r1->nickname, fmt_addr32(r1->addr),
               r1->or_port, r1->dir_port);
  tor_assert(line);

  return line;
}

/* Allocate and return a new string containing a "platform" line for the
 * current Tor version and OS. */
static char *
get_new_platform_line(void)
{
  char *line = NULL;

  tor_asprintf(&line,
               "platform Tor %s on %s\n",
               VERSION, get_uname());
  tor_assert(line);

  return line;
}

/* Allocate and return a new string containing a "published" line for r1.
 * r1->cache_info.published_on must be between 0 and 59 seconds. */
static char *
get_new_published_line(const routerinfo_t *r1)
{
  char *line = NULL;

  tor_assert(r1);

  tor_assert(r1->cache_info.published_on >= 0);
  tor_assert(r1->cache_info.published_on <= 59);

  tor_asprintf(&line,
               "published 1970-01-01 00:00:%02u\n",
               (unsigned)r1->cache_info.published_on);
  tor_assert(line);

  return line;
}

/* Allocate and return a new string containing a "fingerprint" line for r1. */
static char *
get_new_fingerprint_line(const routerinfo_t *r1)
{
  char *line = NULL;
  char fingerprint[FINGERPRINT_LEN+1];

  tor_assert(r1);

  tor_assert(!crypto_pk_get_fingerprint(r1->identity_pkey, fingerprint, 1));
  tor_assert(strlen(fingerprint) > 0);

  tor_asprintf(&line,
               "fingerprint %s\n",
               fingerprint);
  tor_assert(line);

  return line;
}

/* Allocate and return a new string containing an "uptime" line with uptime t.
 *
 * You should pass a hard-coded value to this function, because even if we made
 * it reflect uptime, that still wouldn't make it right, because the two
 * descriptors might be made on different seconds.
 */
static char *
get_new_uptime_line(time_t t)
{
  char *line = NULL;

  tor_asprintf(&line,
               "uptime %u\n",
               (unsigned)t);
  tor_assert(line);

  return line;
}

/* Allocate and return a new string containing an "bandwidth" line for r1.
 */
static char *
get_new_bandwidth_line(const routerinfo_t *r1)
{
  char *line = NULL;

  tor_assert(r1);

  tor_asprintf(&line,
               "bandwidth %u %u %u\n",
               r1->bandwidthrate,
               r1->bandwidthburst,
               r1->bandwidthcapacity);
  tor_assert(line);

  return line;
}

/* Allocate and return a new string containing a key_name block for the
 * RSA key pk1.
 */
static char *
get_new_rsa_key_block(const char *key_name, crypto_pk_t *pk1)
{
  char *block = NULL;
  char *pk1_str = NULL;
  size_t pk1_str_len = 0;

  tor_assert(key_name);
  tor_assert(pk1);

  tor_assert(!crypto_pk_write_public_key_to_string(pk1, &pk1_str,
                                                   &pk1_str_len));
  tor_assert(pk1_str);
  tor_assert(pk1_str_len);

  tor_asprintf(&block,
               "%s\n%s",
               key_name,
               pk1_str);
  tor_free(pk1_str);

  tor_assert(block);
  return block;
}

/* Allocate and return a new string containing an "onion-key" block for the
 * router r1.
 */
static char *
get_new_onion_key_block(const routerinfo_t *r1)
{
  char *block = NULL;
  tor_assert(r1);
  crypto_pk_t *pk_tmp = router_get_rsa_onion_pkey(r1->onion_pkey,
                                                  r1->onion_pkey_len);
  block = get_new_rsa_key_block("onion-key", pk_tmp);
  crypto_pk_free(pk_tmp);
  return block;
}

/* Allocate and return a new string containing an "signing-key" block for the
 * router r1.
 */
static char *
get_new_signing_key_block(const routerinfo_t *r1)
{
  tor_assert(r1);
  return get_new_rsa_key_block("signing-key", r1->identity_pkey);
}

/* Allocate and return a new string containing an "ntor-onion-key" line for
 * the curve25519 public key ntor_onion_pubkey.
 */
static char *
get_new_ntor_onion_key_line(const curve25519_public_key_t *ntor_onion_pubkey)
{
  char *line = NULL;
  char cert_buf[256];
  int rv = 0;

  tor_assert(ntor_onion_pubkey);

  rv = base64_encode(cert_buf, sizeof(cert_buf),
                     (const char*)ntor_onion_pubkey->public_key, 32,
                     BASE64_ENCODE_MULTILINE);
  tor_assert(rv > 0);
  tor_assert(strlen(cert_buf) > 0);

  tor_asprintf(&line,
               "ntor-onion-key %s",
               cert_buf);
  tor_assert(line);

  return line;
}

/* Allocate and return a new string containing a "bridge-distribution-request"
 * line for options.
 */
static char *
get_new_bridge_distribution_request_line(const or_options_t *options)
{
  if (options->BridgeRelay) {
    return tor_strdup("bridge-distribution-request any\n");
  } else {
    return tor_strdup("");
  }
}

static smartlist_t *mocked_configured_ports = NULL;

/** Returns mocked_configured_ports */
static const smartlist_t *
mock_get_configured_ports(void)
{
  return mocked_configured_ports;
}

static tor_cert_t *
mock_tor_cert_dup_null(const tor_cert_t *cert)
{
  (void)cert;
  return NULL;
}

static crypto_pk_t *mocked_server_identitykey = NULL;

/* Returns mocked_server_identitykey with no checks. */
static crypto_pk_t *
mock_get_server_identity_key(void)
{
  return mocked_server_identitykey;
}

static crypto_pk_t *mocked_onionkey = NULL;

/* Returns mocked_onionkey with no checks. */
static crypto_pk_t *
mock_get_onion_key(void)
{
  return mocked_onionkey;
}

static routerinfo_t *mocked_routerinfo = NULL;

/* Returns 0 and sets ri_out to mocked_routerinfo.
 * ri_out must not be NULL. There are no other checks. */
static int
mock_router_build_fresh_unsigned_routerinfo(routerinfo_t **ri_out)
{
  tor_assert(ri_out);
  *ri_out = mocked_routerinfo;
  return 0;
}

static ed25519_keypair_t *mocked_master_signing_key = NULL;

/* Returns mocked_master_signing_key with no checks. */
static const ed25519_keypair_t *
mock_get_master_signing_keypair(void)
{
  return mocked_master_signing_key;
}

static struct tor_cert_st *mocked_signing_key_cert = NULL;

/* Returns mocked_signing_key_cert with no checks. */
static const struct tor_cert_st *
mock_get_master_signing_key_cert(void)
{
  return mocked_signing_key_cert;
}

static curve25519_keypair_t *mocked_curve25519_onion_key = NULL;

/* Returns mocked_curve25519_onion_key with no checks. */
static const curve25519_keypair_t *
mock_get_current_curve25519_keypair(void)
{
  return mocked_curve25519_onion_key;
}

/* Unmock get_configured_ports() and free mocked_configured_ports. */
static void
cleanup_mock_configured_ports(void)
{
  UNMOCK(get_configured_ports);

  if (mocked_configured_ports) {
    SMARTLIST_FOREACH(mocked_configured_ports, port_cfg_t *, p, tor_free(p));
    smartlist_free(mocked_configured_ports);
  }
}

/* Mock get_configured_ports() with a list containing or_port and dir_port.
 * If a port is 0, don't set it.
 * Only sets the minimal data required for the tests to pass. */
static void
setup_mock_configured_ports(uint16_t or_port, uint16_t dir_port)
{
  cleanup_mock_configured_ports();

  /* Fake just enough of an ORPort and DirPort to get by */
  MOCK(get_configured_ports, mock_get_configured_ports);
  mocked_configured_ports = smartlist_new();

  if (or_port) {
    port_cfg_t *or_port_cfg = tor_malloc_zero(sizeof(*or_port_cfg));
    or_port_cfg->type = CONN_TYPE_OR_LISTENER;
    or_port_cfg->addr.family = AF_INET;
    or_port_cfg->port = or_port;
    smartlist_add(mocked_configured_ports, or_port_cfg);
  }

  if (dir_port) {
    port_cfg_t *dir_port_cfg = tor_malloc_zero(sizeof(*dir_port_cfg));
    dir_port_cfg->type = CONN_TYPE_DIR_LISTENER;
    dir_port_cfg->addr.family = AF_INET;
    dir_port_cfg->port = dir_port;
    smartlist_add(mocked_configured_ports, dir_port_cfg);
  }
}

/* Clean up the data structures and unmock the functions needed for generating
 * a fresh descriptor. */
static void
cleanup_mocks_for_fresh_descriptor(void)
{
  tor_free(get_options_mutable()->Nickname);

  mocked_server_identitykey = NULL;
  UNMOCK(get_server_identity_key);

  crypto_pk_free(mocked_onionkey);
  UNMOCK(get_onion_key);
}

/* Mock the data structures and functions needed for generating a fresh
 * descriptor.
 *
 * Sets options->Nickname from r1->nickname.
 * Mocks get_server_identity_key() with r1->identity_pkey.
 *
 * If rsa_onion_keypair is not NULL, it is used to mock get_onion_key().
 * Otherwise, the public key in r1->onion_pkey is used to mock get_onion_key().
 */
static void
setup_mocks_for_fresh_descriptor(const routerinfo_t *r1,
                                 crypto_pk_t *rsa_onion_keypair)
{
  cleanup_mocks_for_fresh_descriptor();

  tor_assert(r1);

  /* router_build_fresh_signed_extrainfo() requires options->Nickname */
  get_options_mutable()->Nickname = tor_strdup(r1->nickname);

  /* router_build_fresh_signed_extrainfo() requires get_server_identity_key().
   * Use the same one as the call to router_dump_router_to_string() above.
   */
  mocked_server_identitykey = r1->identity_pkey;
  MOCK(get_server_identity_key, mock_get_server_identity_key);

  /* router_dump_and_sign_routerinfo_descriptor_body() requires
   * get_onion_key(). Use the same one as r1.
   */
  if (rsa_onion_keypair) {
    mocked_onionkey = crypto_pk_dup_key(rsa_onion_keypair);
  } else {
    mocked_onionkey = router_get_rsa_onion_pkey(r1->onion_pkey,
                                                r1->onion_pkey_len);
  }
  MOCK(get_onion_key, mock_get_onion_key);
}

/* Set options based on arg.
 *
 * b: BridgeRelay 1
 * e: ExtraInfoStatistics 1
 * s: sets all the individual statistics options to 1
 *
 * Always sets AssumeReachable to 1.
 *
 * Does not set ServerTransportPlugin, because it's parsed before use.
 *
 * Does not set BridgeRecordUsageByCountry, because the tests don't have access
 * to a GeoIPFile or GeoIPv6File. */
static void
setup_dir_formats_options(const char *arg, or_options_t *options)
{
  /* Skip reachability checks for DirPort, ORPort, and tunnelled-dir-server */
  options->AssumeReachable = 1;

  if (strchr(arg, 'b')) {
    options->BridgeRelay = 1;
  }

  if (strchr(arg, 'e')) {
    options->ExtraInfoStatistics = 1;
  }

  if (strchr(arg, 's')) {
    options->DirReqStatistics = 1;
    options->HiddenServiceStatistics = 1;
    options->EntryStatistics = 1;
    options->CellStatistics = 1;
    options->ExitPortStatistics = 1;
    options->ConnDirectionStatistics = 1;
    options->PaddingStatistics = 1;
  }
}

/* Check that routerinfos r1 and rp1 are consistent.
 * Only performs some basic checks.
 */
#define CHECK_ROUTERINFO_CONSISTENCY(r1, rp1) \
STMT_BEGIN \
  tt_assert(r1); \
  tt_assert(rp1); \
\
  tt_int_op(rp1->addr,OP_EQ, r1->addr); \
  tt_int_op(rp1->or_port,OP_EQ, r1->or_port); \
  tt_int_op(rp1->dir_port,OP_EQ, r1->dir_port); \
  tt_int_op(rp1->bandwidthrate,OP_EQ, r1->bandwidthrate); \
  tt_int_op(rp1->bandwidthburst,OP_EQ, r1->bandwidthburst); \
  tt_int_op(rp1->bandwidthcapacity,OP_EQ, r1->bandwidthcapacity); \
  crypto_pk_t *rp1_onion_pkey = router_get_rsa_onion_pkey(rp1->onion_pkey, \
                                                      rp1->onion_pkey_len); \
  crypto_pk_t *r1_onion_pkey = router_get_rsa_onion_pkey(r1->onion_pkey, \
                                                      r1->onion_pkey_len); \
  tt_int_op(crypto_pk_cmp_keys(rp1_onion_pkey, r1_onion_pkey), OP_EQ, 0); \
  crypto_pk_free(rp1_onion_pkey); \
  crypto_pk_free(r1_onion_pkey); \
  tt_int_op(crypto_pk_cmp_keys(rp1->identity_pkey, r1->identity_pkey), \
            OP_EQ, 0); \
  tt_int_op(rp1->supports_tunnelled_dir_requests, OP_EQ, \
            r1->supports_tunnelled_dir_requests); \
STMT_END

/* Check that routerinfo r1 and extrainfo e1 are consistent.
 * Only performs some basic checks.
 */
#define CHECK_EXTRAINFO_CONSISTENCY(r1, e1) \
STMT_BEGIN \
  tt_assert(r1); \
  tt_assert(e1); \
\
  tt_str_op(e1->nickname, OP_EQ, r1->nickname); \
STMT_END

/** Run unit tests for router descriptor generation logic for a RSA-only
 * router. Tor versions without ed25519 (0.2.6 and earlier) are no longer
 * officially supported, but the authorities still accept their descriptors.
 */
static void
test_dir_formats_rsa(void *arg)
{
  char *buf = NULL;
  char *buf2 = NULL;
  char *cp = NULL;

  uint8_t *rsa_cc = NULL;

  routerinfo_t *r1 = NULL;
  extrainfo_t *e1 = NULL;
  routerinfo_t *rp1 = NULL;
  extrainfo_t *ep1 = NULL;

  smartlist_t *chunks = NULL;
  const char *msg = NULL;
  int rv = -1;

  or_options_t *options = get_options_mutable();
  setup_dir_formats_options((const char *)arg, options);

  hibernate_set_state_for_testing_(HIBERNATE_STATE_LIVE);

  /* r1 is a minimal, RSA-only descriptor, with DirPort and IPv6 */
  r1 = basic_routerinfo_new("Magri", 0xc0a80001u /* 192.168.0.1 */,
                            9000, 9003,
                            1000, 5000, 10000,
                            0,
                            NULL);

 /* Fake just enough of an ntor key to get by */
  curve25519_keypair_t r1_onion_keypair;
  curve25519_keypair_generate(&r1_onion_keypair, 0);
  r1->onion_curve25519_pkey = tor_memdup(&r1_onion_keypair.pubkey,
                                         sizeof(curve25519_public_key_t));

  /* Now add IPv6 */
  tor_addr_parse(&r1->ipv6_addr, "1:2:3:4::");
  r1->ipv6_orport = 9999;

  r1->exit_policy = NULL;

  /* XXXX+++ router_dump_to_string should really take this from ri. */
  options->ContactInfo = tor_strdup("Magri White "
                                    "<magri@elsewhere.example.com>");

  setup_mock_configured_ports(r1->or_port, r1->dir_port);

  buf = router_dump_router_to_string(r1, r1->identity_pkey, NULL, NULL, NULL);
  tt_assert(buf);

  tor_free(options->ContactInfo);
  cleanup_mock_configured_ports();

  /* Synthesise a router descriptor, without the signature */
  chunks = smartlist_new();

  smartlist_add(chunks, get_new_router_line(r1));
  smartlist_add_strdup(chunks, "or-address [1:2:3:4::]:9999\n");

  smartlist_add(chunks, get_new_platform_line());
  smartlist_add(chunks, get_new_published_line(r1));
  smartlist_add(chunks, get_new_fingerprint_line(r1));

  smartlist_add(chunks, get_new_uptime_line(0));
  smartlist_add(chunks, get_new_bandwidth_line(r1));

  smartlist_add(chunks, get_new_onion_key_block(r1));
  smartlist_add(chunks, get_new_signing_key_block(r1));

  smartlist_add_strdup(chunks, "hidden-service-dir\n");

  smartlist_add_strdup(chunks, "contact Magri White "
                               "<magri@elsewhere.example.com>\n");

  smartlist_add(chunks, get_new_bridge_distribution_request_line(options));
  smartlist_add(chunks, get_new_ntor_onion_key_line(&r1_onion_keypair.pubkey));
  smartlist_add_strdup(chunks, "reject *:*\n");
  smartlist_add_strdup(chunks, "tunnelled-dir-server\n");

  smartlist_add_strdup(chunks, "router-signature\n");

  size_t len_out = 0;
  buf2 = smartlist_join_strings(chunks, "", 0, &len_out);
  SMARTLIST_FOREACH(chunks, char *, s, tor_free(s));
  smartlist_free(chunks);

  tt_assert(len_out > 0);

  buf[strlen(buf2)] = '\0'; /* Don't compare the sig; it's never the same
                             * twice */

  tt_str_op(buf,OP_EQ, buf2);
  tor_free(buf);

  setup_mock_configured_ports(r1->or_port, r1->dir_port);

  buf = router_dump_router_to_string(r1, r1->identity_pkey, NULL, NULL, NULL);
  tt_assert(buf);

  cleanup_mock_configured_ports();

  /* Now, try to parse buf */
  cp = buf;
  rp1 = router_parse_entry_from_string((const char*)cp,NULL,1,0,NULL,NULL);

  CHECK_ROUTERINFO_CONSISTENCY(r1, rp1);

  tt_assert(rp1->policy_is_reject_star);

  tor_free(buf);
  routerinfo_free(rp1);

  /* Test extrainfo creation.
   * We avoid calling router_build_fresh_unsigned_routerinfo(), because it's
   * too complex. Instead, we re-use the manually-created routerinfos.
   */

  /* Set up standard mocks and data */
  setup_mocks_for_fresh_descriptor(r1, NULL);

  /* router_build_fresh_signed_extrainfo() passes the result of
   * get_master_signing_key_cert() directly to tor_cert_dup(), which fails on
   * NULL. But we want a NULL ei->cache_info.signing_key_cert to test the
   * non-ed key path.
   */
  MOCK(tor_cert_dup, mock_tor_cert_dup_null);

  /* Fake just enough of an ORPort and DirPort to get by */
  setup_mock_configured_ports(r1->or_port, r1->dir_port);

  /* Test some of the low-level static functions. */
  e1 = router_build_fresh_signed_extrainfo(r1);
  tt_assert(e1);
  router_update_routerinfo_from_extrainfo(r1, e1);
  rv = router_dump_and_sign_routerinfo_descriptor_body(r1);
  tt_assert(rv == 0);
  msg = "";
  rv = routerinfo_incompatible_with_extrainfo(r1->identity_pkey, e1,
                                              &r1->cache_info, &msg);
  /* If they are incompatible, fail and show the msg string */
  tt_str_op(msg, OP_EQ, "");
  tt_assert(rv == 0);

  /* Now cleanup */
  cleanup_mocks_for_fresh_descriptor();

  UNMOCK(tor_cert_dup);

  cleanup_mock_configured_ports();

  CHECK_EXTRAINFO_CONSISTENCY(r1, e1);

  /* Test that the signed ri is parseable */
  tt_assert(r1->cache_info.signed_descriptor_body);
  cp = r1->cache_info.signed_descriptor_body;
  rp1 = router_parse_entry_from_string((const char*)cp,NULL,1,0,NULL,NULL);

  CHECK_ROUTERINFO_CONSISTENCY(r1, rp1);

  tt_assert(rp1->policy_is_reject_star);

  routerinfo_free(rp1);

  /* Test that the signed ei is parseable */
  tt_assert(e1->cache_info.signed_descriptor_body);
  cp = e1->cache_info.signed_descriptor_body;
  ep1 = extrainfo_parse_entry_from_string((const char*)cp,NULL,1,NULL,NULL);

  CHECK_EXTRAINFO_CONSISTENCY(r1, ep1);

  /* In future tests, we could check the actual extrainfo statistics. */

  extrainfo_free(ep1);

 done:
  dirserv_free_fingerprint_list();

  tor_free(options->ContactInfo);
  tor_free(options->Nickname);

  cleanup_mock_configured_ports();
  cleanup_mocks_for_fresh_descriptor();

  if (chunks) {
    SMARTLIST_FOREACH(chunks, char *, s, tor_free(s));
    smartlist_free(chunks);
  }

  routerinfo_free(r1);
  routerinfo_free(rp1);

  extrainfo_free(e1);
  extrainfo_free(ep1);

  tor_free(rsa_cc);

  tor_free(buf);
  tor_free(buf2);
}

/* Check that the exit policy in rp2 is as expected. */
#define CHECK_PARSED_EXIT_POLICY(rp2) \
STMT_BEGIN \
  tt_int_op(smartlist_len(rp2->exit_policy),OP_EQ, 2); \
 \
  p = smartlist_get(rp2->exit_policy, 0); \
  tt_int_op(p->policy_type,OP_EQ, ADDR_POLICY_ACCEPT); \
  tt_assert(tor_addr_is_null(&p->addr)); \
  tt_int_op(p->maskbits,OP_EQ, 0); \
  tt_int_op(p->prt_min,OP_EQ, 80); \
  tt_int_op(p->prt_max,OP_EQ, 80); \
 \
  p = smartlist_get(rp2->exit_policy, 1); \
  tt_int_op(p->policy_type,OP_EQ, ADDR_POLICY_REJECT); \
  tt_assert(tor_addr_eq(&p->addr, &ex2->addr)); \
  tt_int_op(p->maskbits,OP_EQ, 8); \
  tt_int_op(p->prt_min,OP_EQ, 24); \
  tt_int_op(p->prt_max,OP_EQ, 24); \
STMT_END

/** Run unit tests for router descriptor generation logic for a RSA + ed25519
 * router.
 */
static void
test_dir_formats_rsa_ed25519(void *arg)
{
  char *buf = NULL;
  char *buf2 = NULL;
  char *cp = NULL;

  crypto_pk_t *r2_onion_pkey = NULL;
  char cert_buf[256];
  uint8_t *rsa_cc = NULL;
  time_t now = time(NULL);

  routerinfo_t *r2 = NULL;
  extrainfo_t *e2 = NULL;
  routerinfo_t *r2_out = NULL;
  routerinfo_t *rp2 = NULL;
  extrainfo_t *ep2 = NULL;
  addr_policy_t *ex1, *ex2;
  const addr_policy_t *p;

  smartlist_t *chunks = NULL;
  int rv = -1;

  or_options_t *options = get_options_mutable();
  setup_dir_formats_options((const char *)arg, options);

  hibernate_set_state_for_testing_(HIBERNATE_STATE_LIVE);

  /* r2 is a RSA + ed25519 descriptor, with an exit policy, but no DirPort or
   * IPv6 */
  r2 = basic_routerinfo_new("Fred", 0x0a030201u /* 10.3.2.1 */,
                            9005, 0,
                            3000, 3000, 3000,
                            5,
                            &r2_onion_pkey);

  /* Fake just enough of an ntor key to get by */
  curve25519_keypair_t r2_onion_keypair;
  curve25519_keypair_generate(&r2_onion_keypair, 0);
  r2->onion_curve25519_pkey = tor_memdup(&r2_onion_keypair.pubkey,
                                         sizeof(curve25519_public_key_t));

  /* Now add relay ed25519 keys
   * We can't use init_mock_ed_keys() here, because the keys are seeded */
  ed25519_keypair_t kp1, kp2;
  ed25519_secret_key_from_seed(&kp1.seckey,
                          (const uint8_t*)"YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY");
  ed25519_public_key_generate(&kp1.pubkey, &kp1.seckey);
  ed25519_secret_key_from_seed(&kp2.seckey,
                          (const uint8_t*)"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
  ed25519_public_key_generate(&kp2.pubkey, &kp2.seckey);
  r2->cache_info.signing_key_cert = tor_cert_create(&kp1,
                                         CERT_TYPE_ID_SIGNING,
                                         &kp2.pubkey,
                                         now, 86400,
                                         CERT_FLAG_INCLUDE_SIGNING_KEY);

  /* Now add an exit policy */
  ex1 = tor_malloc_zero(sizeof(addr_policy_t));
  ex2 = tor_malloc_zero(sizeof(addr_policy_t));
  ex1->policy_type = ADDR_POLICY_ACCEPT;
  tor_addr_from_ipv4h(&ex1->addr, 0);
  ex1->maskbits = 0;
  ex1->prt_min = ex1->prt_max = 80;
  ex2->policy_type = ADDR_POLICY_REJECT;
  tor_addr_from_ipv4h(&ex2->addr, 18<<24);
  ex2->maskbits = 8;
  ex2->prt_min = ex2->prt_max = 24;

  r2->exit_policy = smartlist_new();
  smartlist_add(r2->exit_policy, ex1);
  smartlist_add(r2->exit_policy, ex2);

  /* Fake just enough of an ORPort to get by */
  setup_mock_configured_ports(r2->or_port, 0);

  buf = router_dump_router_to_string(r2,
                                     r2->identity_pkey, r2_onion_pkey,
                                     &r2_onion_keypair, &kp2);
  tt_assert(buf);

  cleanup_mock_configured_ports();

  chunks = smartlist_new();

  /* Synthesise a router descriptor, without the signatures */
  smartlist_add(chunks, get_new_router_line(r2));

  smartlist_add_strdup(chunks,
                       "identity-ed25519\n"
                       "-----BEGIN ED25519 CERT-----\n");
  base64_encode(cert_buf, sizeof(cert_buf),
                (const char*)r2->cache_info.signing_key_cert->encoded,
                r2->cache_info.signing_key_cert->encoded_len,
                BASE64_ENCODE_MULTILINE);
  smartlist_add_strdup(chunks, cert_buf);
  smartlist_add_strdup(chunks, "-----END ED25519 CERT-----\n");

  smartlist_add_strdup(chunks, "master-key-ed25519 ");
  {
    char k[ED25519_BASE64_LEN+1];
    ed25519_public_to_base64(k, &r2->cache_info.signing_key_cert->signing_key);
    smartlist_add_strdup(chunks, k);
    smartlist_add_strdup(chunks, "\n");
  }

  smartlist_add(chunks, get_new_platform_line());
  smartlist_add(chunks, get_new_published_line(r2));
  smartlist_add(chunks, get_new_fingerprint_line(r2));

  smartlist_add(chunks, get_new_uptime_line(0));
  smartlist_add(chunks, get_new_bandwidth_line(r2));

  smartlist_add(chunks, get_new_onion_key_block(r2));
  smartlist_add(chunks, get_new_signing_key_block(r2));

  int rsa_cc_len;
  rsa_cc = make_tap_onion_key_crosscert(r2_onion_pkey,
                                        &kp1.pubkey,
                                        r2->identity_pkey,
                                        &rsa_cc_len);
  tt_assert(rsa_cc);
  base64_encode(cert_buf, sizeof(cert_buf), (char*)rsa_cc, rsa_cc_len,
                BASE64_ENCODE_MULTILINE);
  smartlist_add_strdup(chunks, "onion-key-crosscert\n"
                       "-----BEGIN CROSSCERT-----\n");
  smartlist_add_strdup(chunks, cert_buf);
  smartlist_add_strdup(chunks, "-----END CROSSCERT-----\n");
  int ntor_cc_sign;
  {
    tor_cert_t *ntor_cc = NULL;
    ntor_cc = make_ntor_onion_key_crosscert(&r2_onion_keypair,
                                          &kp1.pubkey,
                                          r2->cache_info.published_on,
                                          get_onion_key_lifetime(),
                                          &ntor_cc_sign);
    tt_assert(ntor_cc);
    base64_encode(cert_buf, sizeof(cert_buf),
                (char*)ntor_cc->encoded, ntor_cc->encoded_len,
                BASE64_ENCODE_MULTILINE);
    tor_cert_free(ntor_cc);
  }
  smartlist_add_asprintf(chunks,
               "ntor-onion-key-crosscert %d\n"
               "-----BEGIN ED25519 CERT-----\n"
               "%s"
               "-----END ED25519 CERT-----\n", ntor_cc_sign, cert_buf);

  smartlist_add_strdup(chunks, "hidden-service-dir\n");

  smartlist_add(chunks, get_new_bridge_distribution_request_line(options));
  smartlist_add(chunks, get_new_ntor_onion_key_line(&r2_onion_keypair.pubkey));
  smartlist_add_strdup(chunks, "accept *:80\nreject 18.0.0.0/8:24\n");
  smartlist_add_strdup(chunks, "tunnelled-dir-server\n");

  smartlist_add_strdup(chunks, "router-sig-ed25519 ");

  size_t len_out = 0;
  buf2 = smartlist_join_strings(chunks, "", 0, &len_out);
  SMARTLIST_FOREACH(chunks, char *, s, tor_free(s));
  smartlist_free(chunks);

  tt_assert(len_out > 0);

  buf[strlen(buf2)] = '\0'; /* Don't compare either sig; they're never the same
                             * twice */

  tt_str_op(buf, OP_EQ, buf2);
  tor_free(buf);

  setup_mock_configured_ports(r2->or_port, 0);

  buf = router_dump_router_to_string(r2, r2->identity_pkey, NULL, NULL, NULL);
  tt_assert(buf);

  cleanup_mock_configured_ports();

  /* Now, try to parse buf */
  cp = buf;
  rp2 = router_parse_entry_from_string((const char*)cp,NULL,1,0,NULL,NULL);

  CHECK_ROUTERINFO_CONSISTENCY(r2, rp2);

  tt_mem_op(rp2->onion_curve25519_pkey->public_key,OP_EQ,
             r2->onion_curve25519_pkey->public_key,
             CURVE25519_PUBKEY_LEN);

  CHECK_PARSED_EXIT_POLICY(rp2);

  tor_free(buf);
  routerinfo_free(rp2);

  /* Test extrainfo creation. */

  /* Set up standard mocks and data */
  setup_mocks_for_fresh_descriptor(r2, r2_onion_pkey);

  /* router_build_fresh_descriptor() requires
   * router_build_fresh_unsigned_routerinfo(), but the implementation is
   * too complex. Instead, we re-use r2.
   */
  mocked_routerinfo = r2;
  MOCK(router_build_fresh_unsigned_routerinfo,
       mock_router_build_fresh_unsigned_routerinfo);

  /* r2 uses ed25519, so we need to mock the ed key functions */
  mocked_master_signing_key = &kp2;
  MOCK(get_master_signing_keypair, mock_get_master_signing_keypair);

  mocked_signing_key_cert = r2->cache_info.signing_key_cert;
  MOCK(get_master_signing_key_cert, mock_get_master_signing_key_cert);

  mocked_curve25519_onion_key = &r2_onion_keypair;
  MOCK(get_current_curve25519_keypair, mock_get_current_curve25519_keypair);

  /* Fake just enough of an ORPort to get by */
  setup_mock_configured_ports(r2->or_port, 0);

  /* Test the high-level interface. */
  rv = router_build_fresh_descriptor(&r2_out, &e2);
  if (rv < 0) {
    /* router_build_fresh_descriptor() frees r2 on failure. */
    r2 = NULL;
    /* Get rid of an alias to rp2 */
    r2_out = NULL;
  }
  tt_assert(rv == 0);
  tt_assert(r2_out);
  tt_assert(e2);
  /* Guaranteed by mock_router_build_fresh_unsigned_routerinfo() */
  tt_ptr_op(r2_out, OP_EQ, r2);
  /* Get rid of an alias to r2 */
  r2_out = NULL;

  /* Now cleanup */
  cleanup_mocks_for_fresh_descriptor();

  mocked_routerinfo = NULL;
  UNMOCK(router_build_fresh_unsigned_routerinfo);
  mocked_master_signing_key = NULL;
  UNMOCK(get_master_signing_keypair);
  mocked_signing_key_cert = NULL;
  UNMOCK(get_master_signing_key_cert);
  mocked_curve25519_onion_key = NULL;
  UNMOCK(get_current_curve25519_keypair);

  cleanup_mock_configured_ports();

  CHECK_EXTRAINFO_CONSISTENCY(r2, e2);

  /* Test that the signed ri is parseable */
  tt_assert(r2->cache_info.signed_descriptor_body);
  cp = r2->cache_info.signed_descriptor_body;
  rp2 = router_parse_entry_from_string((const char*)cp,NULL,1,0,NULL,NULL);

  CHECK_ROUTERINFO_CONSISTENCY(r2, rp2);

  tt_mem_op(rp2->onion_curve25519_pkey->public_key,OP_EQ,
            r2->onion_curve25519_pkey->public_key,
            CURVE25519_PUBKEY_LEN);

  CHECK_PARSED_EXIT_POLICY(rp2);

  routerinfo_free(rp2);

  /* Test that the signed ei is parseable */
  tt_assert(e2->cache_info.signed_descriptor_body);
  cp = e2->cache_info.signed_descriptor_body;
  ep2 = extrainfo_parse_entry_from_string((const char*)cp,NULL,1,NULL,NULL);

  CHECK_EXTRAINFO_CONSISTENCY(r2, ep2);

  /* In future tests, we could check the actual extrainfo statistics. */

  extrainfo_free(ep2);

 done:
  dirserv_free_fingerprint_list();

  tor_free(options->Nickname);

  cleanup_mock_configured_ports();
  cleanup_mocks_for_fresh_descriptor();

  if (chunks) {
    SMARTLIST_FOREACH(chunks, char *, s, tor_free(s));
    smartlist_free(chunks);
  }

  routerinfo_free(r2);
  routerinfo_free(r2_out);
  routerinfo_free(rp2);

  extrainfo_free(e2);
  extrainfo_free(ep2);

  tor_free(rsa_cc);
  crypto_pk_free(r2_onion_pkey);

  tor_free(buf);
  tor_free(buf2);
}

#include "failing_routerdescs.inc"

static void
test_dir_routerinfo_parsing(void *arg)
{
  (void) arg;

  int again;
  routerinfo_t *ri = NULL;

#define CHECK_OK(s)                                                     \
  do {                                                                  \
    routerinfo_free(ri);                                                \
    ri = router_parse_entry_from_string((s), NULL, 0, 0, NULL, NULL);   \
    tt_assert(ri);                                                      \
  } while (0)
#define CHECK_FAIL(s, againval)                                         \
  do {                                                                  \
    routerinfo_free(ri);                                                \
    again = 999;                                                        \
    ri = router_parse_entry_from_string((s), NULL, 0, 0, NULL, &again); \
    tt_assert(ri == NULL);                                              \
    tt_int_op(again, OP_EQ, (againval));                                   \
  } while (0)

  CHECK_OK(EX_RI_MINIMAL);
  CHECK_OK(EX_RI_MAXIMAL);

  CHECK_OK(EX_RI_MINIMAL_ED);

  /* good annotations prepended */
  routerinfo_free(ri);
  ri = router_parse_entry_from_string(EX_RI_MINIMAL, NULL, 0, 0,
                                      "@purpose bridge\n", NULL);
  tt_ptr_op(ri, OP_NE, NULL);
  tt_assert(ri->purpose == ROUTER_PURPOSE_BRIDGE);
  routerinfo_free(ri);

  /* bad annotations prepended. */
  ri = router_parse_entry_from_string(EX_RI_MINIMAL,
                                      NULL, 0, 0, "@purpose\n", NULL);
  tt_ptr_op(ri, OP_EQ, NULL);

  /* bad annotations on router. */
  ri = router_parse_entry_from_string("@purpose\nrouter x\n", NULL, 0, 1,
                                      NULL, NULL);
  tt_ptr_op(ri, OP_EQ, NULL);

  /* unwanted annotations on router. */
  ri = router_parse_entry_from_string("@purpose foo\nrouter x\n", NULL, 0, 0,
                                      NULL, NULL);
  tt_ptr_op(ri, OP_EQ, NULL);

  /* No signature. */
  ri = router_parse_entry_from_string("router x\n", NULL, 0, 0,
                                      NULL, NULL);
  tt_ptr_op(ri, OP_EQ, NULL);

  /* Not a router */
  routerinfo_free(ri);
  ri = router_parse_entry_from_string("hello\n", NULL, 0, 0, NULL, NULL);
  tt_ptr_op(ri, OP_EQ, NULL);

  CHECK_FAIL(EX_RI_BAD_SIG1, 1);
  CHECK_FAIL(EX_RI_BAD_SIG2, 1);
  CHECK_FAIL(EX_RI_BAD_TOKENS, 0);
  CHECK_FAIL(EX_RI_BAD_PUBLISHED, 0);
  CHECK_FAIL(EX_RI_NEG_BANDWIDTH, 0);
  CHECK_FAIL(EX_RI_BAD_BANDWIDTH, 0);
  CHECK_FAIL(EX_RI_BAD_BANDWIDTH2, 0);
  CHECK_FAIL(EX_RI_BAD_ONIONKEY1, 0);
  CHECK_FAIL(EX_RI_BAD_ONIONKEY2, 0);
  CHECK_FAIL(EX_RI_BAD_PORTS, 0);
  CHECK_FAIL(EX_RI_BAD_IP, 0);
  CHECK_FAIL(EX_RI_BAD_DIRPORT, 0);
  CHECK_FAIL(EX_RI_BAD_NAME2, 0);
  CHECK_FAIL(EX_RI_BAD_UPTIME, 0);

  CHECK_FAIL(EX_RI_BAD_BANDWIDTH3, 0);
  CHECK_FAIL(EX_RI_BAD_NTOR_KEY, 0);
  CHECK_FAIL(EX_RI_BAD_FINGERPRINT, 0);
  CHECK_FAIL(EX_RI_MISMATCHED_FINGERPRINT, 0);
  CHECK_FAIL(EX_RI_BAD_HAS_ACCEPT6, 0);
  CHECK_FAIL(EX_RI_BAD_NO_EXIT_POLICY, 0);
  CHECK_FAIL(EX_RI_BAD_IPV6_EXIT_POLICY, 0);
  CHECK_FAIL(EX_RI_BAD_FAMILY, 0);
  CHECK_FAIL(EX_RI_ZERO_ORPORT, 0);

  CHECK_FAIL(EX_RI_ED_MISSING_CROSSCERT, 0);
  CHECK_FAIL(EX_RI_ED_MISSING_CROSSCERT2, 0);
  CHECK_FAIL(EX_RI_ED_MISSING_CROSSCERT_SIGN, 0);
  CHECK_FAIL(EX_RI_ED_BAD_SIG1, 0);
  CHECK_FAIL(EX_RI_ED_BAD_SIG2, 0);
  CHECK_FAIL(EX_RI_ED_BAD_SIG3, 0);
  CHECK_FAIL(EX_RI_ED_BAD_SIG4, 0);
  CHECK_FAIL(EX_RI_ED_BAD_CROSSCERT1, 0);
  CHECK_FAIL(EX_RI_ED_BAD_CROSSCERT3, 0);
  CHECK_FAIL(EX_RI_ED_BAD_CROSSCERT4, 0);
  CHECK_FAIL(EX_RI_ED_BAD_CROSSCERT5, 0);
  CHECK_FAIL(EX_RI_ED_BAD_CROSSCERT6, 0);
  CHECK_FAIL(EX_RI_ED_BAD_CROSSCERT7, 0);
  CHECK_FAIL(EX_RI_ED_MISPLACED1, 0);
  CHECK_FAIL(EX_RI_ED_MISPLACED2, 0);
  CHECK_FAIL(EX_RI_ED_BAD_CERT1, 0);
  CHECK_FAIL(EX_RI_ED_BAD_CERT2, 0);
  CHECK_FAIL(EX_RI_ED_BAD_CERT3, 0);

  /* This is allowed; we just ignore it. */
  CHECK_OK(EX_RI_BAD_EI_DIGEST);
  CHECK_OK(EX_RI_BAD_EI_DIGEST2);

#undef CHECK_FAIL
#undef CHECK_OK
 done:
  routerinfo_free(ri);
}

#include "example_extrainfo.inc"

static void
routerinfo_free_wrapper_(void *arg)
{
  routerinfo_free_(arg);
}

static void
test_dir_extrainfo_parsing(void *arg)
{
  (void) arg;

#define CHECK_OK(s)                                                     \
  do {                                                                  \
    extrainfo_free(ei);                                                 \
    ei = extrainfo_parse_entry_from_string((s), NULL, 0, map, NULL);    \
    tt_assert(ei);                                                      \
  } while (0)
#define CHECK_FAIL(s, againval)                                         \
  do {                                                                  \
    extrainfo_free(ei);                                                 \
    again = 999;                                                        \
    ei = extrainfo_parse_entry_from_string((s), NULL, 0, map, &again);  \
    tt_assert(ei == NULL);                                              \
    tt_int_op(again, OP_EQ, (againval));                                   \
  } while (0)
#define ADD(name)                                                       \
  do {                                                                  \
    ri = tor_malloc_zero(sizeof(routerinfo_t));                         \
    crypto_pk_t *pk = ri->identity_pkey = crypto_pk_new();              \
    tt_assert(! crypto_pk_read_public_key_from_string(pk,               \
                                      name##_KEY, strlen(name##_KEY))); \
    tt_int_op(20,OP_EQ,base16_decode(d, 20, name##_FP, strlen(name##_FP))); \
    digestmap_set((digestmap_t*)map, d, ri);                            \
    ri = NULL;                                                          \
  } while (0)

  routerinfo_t *ri = NULL;
  char d[20];
  struct digest_ri_map_t *map = NULL;
  extrainfo_t *ei = NULL;
  int again;

  CHECK_OK(EX_EI_MINIMAL);
  tt_assert(ei->pending_sig);
  CHECK_OK(EX_EI_MAXIMAL);
  tt_assert(ei->pending_sig);
  CHECK_OK(EX_EI_GOOD_ED_EI);
  tt_assert(ei->pending_sig);

  map = (struct digest_ri_map_t *)digestmap_new();
  ADD(EX_EI_MINIMAL);
  ADD(EX_EI_MAXIMAL);
  ADD(EX_EI_GOOD_ED_EI);
  ADD(EX_EI_BAD_FP);
  ADD(EX_EI_BAD_NICKNAME);
  ADD(EX_EI_BAD_TOKENS);
  ADD(EX_EI_BAD_START);
  ADD(EX_EI_BAD_PUBLISHED);

  ADD(EX_EI_ED_MISSING_SIG);
  ADD(EX_EI_ED_MISSING_CERT);
  ADD(EX_EI_ED_BAD_CERT1);
  ADD(EX_EI_ED_BAD_CERT2);
  ADD(EX_EI_ED_BAD_SIG1);
  ADD(EX_EI_ED_BAD_SIG2);
  ADD(EX_EI_ED_MISPLACED_CERT);
  ADD(EX_EI_ED_MISPLACED_SIG);

  CHECK_OK(EX_EI_MINIMAL);
  tt_ptr_op(ei->pending_sig, OP_EQ, NULL);
  CHECK_OK(EX_EI_MAXIMAL);
  tt_ptr_op(ei->pending_sig, OP_EQ, NULL);
  CHECK_OK(EX_EI_GOOD_ED_EI);
  tt_ptr_op(ei->pending_sig, OP_EQ, NULL);

  CHECK_FAIL(EX_EI_BAD_SIG1,1);
  CHECK_FAIL(EX_EI_BAD_SIG2,1);
  CHECK_FAIL(EX_EI_BAD_SIG3,1);
  CHECK_FAIL(EX_EI_BAD_FP,0);
  CHECK_FAIL(EX_EI_BAD_NICKNAME,0);
  CHECK_FAIL(EX_EI_BAD_TOKENS,0);
  CHECK_FAIL(EX_EI_BAD_START,0);
  CHECK_FAIL(EX_EI_BAD_PUBLISHED,0);

  CHECK_FAIL(EX_EI_ED_MISSING_SIG,0);
  CHECK_FAIL(EX_EI_ED_MISSING_CERT,0);
  CHECK_FAIL(EX_EI_ED_BAD_CERT1,0);
  CHECK_FAIL(EX_EI_ED_BAD_CERT2,0);
  CHECK_FAIL(EX_EI_ED_BAD_SIG1,0);
  CHECK_FAIL(EX_EI_ED_BAD_SIG2,0);
  CHECK_FAIL(EX_EI_ED_MISPLACED_CERT,0);
  CHECK_FAIL(EX_EI_ED_MISPLACED_SIG,0);

#undef CHECK_OK
#undef CHECK_FAIL

 done:
  escaped(NULL);
  extrainfo_free(ei);
  routerinfo_free(ri);
  digestmap_free_((digestmap_t*)map, routerinfo_free_wrapper_);
}

static void
test_dir_parse_router_list(void *arg)
{
  (void) arg;
  smartlist_t *invalid = smartlist_new();
  smartlist_t *dest = smartlist_new();
  smartlist_t *chunks = smartlist_new();
  int dest_has_ri = 1;
  char *list = NULL;
  const char *cp;
  digestmap_t *map = NULL;
  char *mem_op_hex_tmp = NULL;
  routerinfo_t *ri = NULL;
  char d[DIGEST_LEN];

  smartlist_add_strdup(chunks, EX_RI_MINIMAL);     // ri 0
  smartlist_add_strdup(chunks, EX_RI_BAD_PORTS);   // bad ri 0
  smartlist_add_strdup(chunks, EX_EI_MAXIMAL);     // ei 0
  smartlist_add_strdup(chunks, EX_EI_BAD_SIG2);    // bad ei --
  smartlist_add_strdup(chunks, EX_EI_BAD_NICKNAME);// bad ei 0
  smartlist_add_strdup(chunks, EX_RI_BAD_SIG1);    // bad ri --
  smartlist_add_strdup(chunks, EX_EI_BAD_PUBLISHED);  // bad ei 1
  smartlist_add_strdup(chunks, EX_RI_MAXIMAL);     // ri 1
  smartlist_add_strdup(chunks, EX_RI_BAD_FAMILY);  // bad ri 1
  smartlist_add_strdup(chunks, EX_EI_MINIMAL);     // ei 1

  list = smartlist_join_strings(chunks, "", 0, NULL);

  /* First, parse the routers. */
  cp = list;
  tt_int_op(0,OP_EQ,
            router_parse_list_from_string(&cp, NULL, dest, SAVED_NOWHERE,
                                          0, 0, NULL, invalid));
  tt_int_op(2, OP_EQ, smartlist_len(dest));
  tt_ptr_op(cp, OP_EQ, list + strlen(list));

  routerinfo_t *r = smartlist_get(dest, 0);
  tt_mem_op(r->cache_info.signed_descriptor_body, OP_EQ,
            EX_RI_MINIMAL, strlen(EX_RI_MINIMAL));
  r = smartlist_get(dest, 1);
  tt_mem_op(r->cache_info.signed_descriptor_body, OP_EQ,
            EX_RI_MAXIMAL, strlen(EX_RI_MAXIMAL));

  tt_int_op(2, OP_EQ, smartlist_len(invalid));
  test_memeq_hex(smartlist_get(invalid, 0),
                 "ab9eeaa95e7d45740185b4e519c76ead756277a9");
  test_memeq_hex(smartlist_get(invalid, 1),
                 "9a651ee03b64325959e8f1b46f2b689b30750b4c");

  /* Now tidy up */
  SMARTLIST_FOREACH(dest, routerinfo_t *, rinfo, routerinfo_free(rinfo));
  SMARTLIST_FOREACH(invalid, uint8_t *, dig, tor_free(dig));
  smartlist_clear(dest);
  smartlist_clear(invalid);

  /* And check extrainfos. */
  dest_has_ri = 0;
  map = (digestmap_t*)router_get_routerlist()->identity_map;
  ADD(EX_EI_MINIMAL);
  ADD(EX_EI_MAXIMAL);
  ADD(EX_EI_BAD_NICKNAME);
  ADD(EX_EI_BAD_PUBLISHED);
  cp = list;
  tt_int_op(0,OP_EQ,
            router_parse_list_from_string(&cp, NULL, dest, SAVED_NOWHERE,
                                          1, 0, NULL, invalid));
  tt_int_op(2, OP_EQ, smartlist_len(dest));
  extrainfo_t *e = smartlist_get(dest, 0);
  tt_mem_op(e->cache_info.signed_descriptor_body, OP_EQ,
            EX_EI_MAXIMAL, strlen(EX_EI_MAXIMAL));
  e = smartlist_get(dest, 1);
  tt_mem_op(e->cache_info.signed_descriptor_body, OP_EQ,
            EX_EI_MINIMAL, strlen(EX_EI_MINIMAL));

  tt_int_op(2, OP_EQ, smartlist_len(invalid));
  test_memeq_hex(smartlist_get(invalid, 0),
                 "d5df4aa62ee9ffc9543d41150c9864908e0390af");
  test_memeq_hex(smartlist_get(invalid, 1),
                 "f61efd2a7f4531f3687a9043e0de90a862ec64ba");

 done:
  tor_free(list);
  if (dest_has_ri)
    SMARTLIST_FOREACH(dest, routerinfo_t *, rt, routerinfo_free(rt));
  else
    SMARTLIST_FOREACH(dest, extrainfo_t *, ei, extrainfo_free(ei));
  smartlist_free(dest);
  SMARTLIST_FOREACH(invalid, uint8_t *, dig, tor_free(dig));
  smartlist_free(invalid);
  SMARTLIST_FOREACH(chunks, char *, chunk, tor_free(chunk));
  smartlist_free(chunks);
  routerinfo_free(ri);
  if (map) {
    digestmap_free_((digestmap_t*)map, routerinfo_free_wrapper_);
    router_get_routerlist()->identity_map =
      (struct digest_ri_map_t*)digestmap_new();
  }
  tor_free(mem_op_hex_tmp);

#undef ADD
}

static download_status_t dls_minimal;
static download_status_t dls_maximal;
static download_status_t dls_bad_fingerprint;
static download_status_t dls_bad_sig2;
static download_status_t dls_bad_ports;
static download_status_t dls_bad_tokens;

static int mock_router_get_dl_status_unrecognized = 0;
static int mock_router_get_dl_status_calls = 0;

static download_status_t *
mock_router_get_dl_status(const char *d)
{
  ++mock_router_get_dl_status_calls;
  char hex[HEX_DIGEST_LEN+1];
  base16_encode(hex, sizeof(hex), d, DIGEST_LEN);
  if (!strcmp(hex, "3E31D19A69EB719C00B02EC60D13356E3F7A3452")) {
    return &dls_minimal;
  } else if (!strcmp(hex, "581D8A368A0FA854ECDBFAB841D88B3F1B004038")) {
    return &dls_maximal;
  } else if (!strcmp(hex, "2578AE227C6116CDE29B3F0E95709B9872DEE5F1")) {
    return &dls_bad_fingerprint;
  } else if (!strcmp(hex, "16D387D3A58F7DB3CF46638F8D0B90C45C7D769C")) {
    return &dls_bad_sig2;
  } else if (!strcmp(hex, "AB9EEAA95E7D45740185B4E519C76EAD756277A9")) {
    return &dls_bad_ports;
  } else if (!strcmp(hex, "A0CC2CEFAD59DBF19F468BFEE60E0868C804B422")) {
    return &dls_bad_tokens;
  } else {
    ++mock_router_get_dl_status_unrecognized;
    return NULL;
  }
}

static void
test_dir_load_routers(void *arg)
{
  (void) arg;
  smartlist_t *chunks = smartlist_new();
  smartlist_t *wanted = smartlist_new();
  char buf[DIGEST_LEN];
  char *mem_op_hex_tmp = NULL;
  char *list = NULL;

#define ADD(str)                                                        \
  do {                                                                  \
    tt_int_op(0,OP_EQ,router_get_router_hash(str, strlen(str), buf));      \
    smartlist_add_strdup(wanted, hex_str(buf, DIGEST_LEN));        \
  } while (0)

  MOCK(router_get_dl_status_by_descriptor_digest, mock_router_get_dl_status);

  update_approx_time(1412510400);

  smartlist_add_strdup(chunks, EX_RI_MINIMAL);
  smartlist_add_strdup(chunks, EX_RI_BAD_FINGERPRINT);
  smartlist_add_strdup(chunks, EX_RI_BAD_SIG2);
  smartlist_add_strdup(chunks, EX_RI_MAXIMAL);
  smartlist_add_strdup(chunks, EX_RI_BAD_PORTS);
  smartlist_add_strdup(chunks, EX_RI_BAD_TOKENS);

  /* not ADDing MINIMIAL */
  ADD(EX_RI_MAXIMAL);
  ADD(EX_RI_BAD_FINGERPRINT);
  ADD(EX_RI_BAD_SIG2);
  /* Not ADDing BAD_PORTS */
  ADD(EX_RI_BAD_TOKENS);

  list = smartlist_join_strings(chunks, "", 0, NULL);
  tt_int_op(1, OP_EQ,
            router_load_routers_from_string(list, NULL, SAVED_IN_JOURNAL,
                                            wanted, 1, NULL));

  /* The "maximal" router was added. */
  /* "minimal" was not. */
  tt_int_op(smartlist_len(router_get_routerlist()->routers),OP_EQ,1);
  routerinfo_t *r = smartlist_get(router_get_routerlist()->routers, 0);
  test_memeq_hex(r->cache_info.signed_descriptor_digest,
                 "581D8A368A0FA854ECDBFAB841D88B3F1B004038");
  tt_int_op(dls_minimal.n_download_failures, OP_EQ, 0);
  tt_int_op(dls_maximal.n_download_failures, OP_EQ, 0);

  /* "Bad fingerprint" and "Bad tokens" should have gotten marked
   * non-retriable. */
  tt_want_int_op(mock_router_get_dl_status_calls, OP_EQ, 2);
  tt_want_int_op(mock_router_get_dl_status_unrecognized, OP_EQ, 0);
  tt_int_op(dls_bad_fingerprint.n_download_failures, OP_EQ, 255);
  tt_int_op(dls_bad_tokens.n_download_failures, OP_EQ, 255);

  /* bad_sig2 and bad ports" are retriable -- one since only the signature
   * was bad, and one because we didn't ask for it. */
  tt_int_op(dls_bad_sig2.n_download_failures, OP_EQ, 0);
  tt_int_op(dls_bad_ports.n_download_failures, OP_EQ, 0);

  /* Wanted still contains "BAD_SIG2" */
  tt_int_op(smartlist_len(wanted), OP_EQ, 1);
  tt_str_op(smartlist_get(wanted, 0), OP_EQ,
            "E0A3753CEFD54128EAB239F294954121DB23D2EF");

#undef ADD

 done:
  tor_free(mem_op_hex_tmp);
  UNMOCK(router_get_dl_status_by_descriptor_digest);
  SMARTLIST_FOREACH(chunks, char *, cp, tor_free(cp));
  smartlist_free(chunks);
  SMARTLIST_FOREACH(wanted, char *, cp, tor_free(cp));
  smartlist_free(wanted);
  tor_free(list);
}

static int mock_get_by_ei_dd_calls = 0;
static int mock_get_by_ei_dd_unrecognized = 0;

static signed_descriptor_t sd_ei_minimal;
static signed_descriptor_t sd_ei_bad_nickname;
static signed_descriptor_t sd_ei_maximal;
static signed_descriptor_t sd_ei_bad_tokens;
static signed_descriptor_t sd_ei_bad_sig2;

static signed_descriptor_t *
mock_get_by_ei_desc_digest(const char *d)
{

  ++mock_get_by_ei_dd_calls;
  char hex[HEX_DIGEST_LEN+1];
  base16_encode(hex, sizeof(hex), d, DIGEST_LEN);

  if (!strcmp(hex, "11E0EDF526950739F7769810FCACAB8C882FAEEE")) {
    return &sd_ei_minimal;
  } else if (!strcmp(hex, "47803B02A0E70E9E8BDA226CB1D74DE354D67DFF")) {
    return &sd_ei_maximal;
  } else if (!strcmp(hex, "D5DF4AA62EE9FFC9543D41150C9864908E0390AF")) {
    return &sd_ei_bad_nickname;
  } else if (!strcmp(hex, "16D387D3A58F7DB3CF46638F8D0B90C45C7D769C")) {
    return &sd_ei_bad_sig2;
  } else if (!strcmp(hex, "9D90F8C42955BBC57D54FB05E54A3F083AF42E8B")) {
    return &sd_ei_bad_tokens;
  } else {
    ++mock_get_by_ei_dd_unrecognized;
    return NULL;
  }
}

static signed_descriptor_t *
mock_ei_get_by_ei_digest(const char *d)
{
  char hex[HEX_DIGEST_LEN+1];
  base16_encode(hex, sizeof(hex), d, DIGEST_LEN);
  signed_descriptor_t *sd = &sd_ei_minimal;

  if (!strcmp(hex, "11E0EDF526950739F7769810FCACAB8C882FAEEE")) {
    sd->signed_descriptor_body = (char *)EX_EI_MINIMAL;
    sd->signed_descriptor_len = sizeof(EX_EI_MINIMAL);
    sd->annotations_len = 0;
    sd->saved_location = SAVED_NOWHERE;
    return sd;
  }
  return NULL;
}

static smartlist_t *mock_ei_insert_list = NULL;
static was_router_added_t
mock_ei_insert(routerlist_t *rl, extrainfo_t *ei, int warn_if_incompatible)
{
  (void) rl;
  (void) warn_if_incompatible;
  smartlist_add(mock_ei_insert_list, ei);
  return ROUTER_ADDED_SUCCESSFULLY;
}

static void
test_dir_load_extrainfo(void *arg)
{
  (void) arg;
  smartlist_t *chunks = smartlist_new();
  smartlist_t *wanted = smartlist_new();
  char buf[DIGEST_LEN];
  char *mem_op_hex_tmp = NULL;
  char *list = NULL;

#define ADD(str)                                                        \
  do {                                                                  \
    tt_int_op(0,OP_EQ,router_get_extrainfo_hash(str, strlen(str), buf));   \
    smartlist_add_strdup(wanted, hex_str(buf, DIGEST_LEN));        \
  } while (0)

  mock_ei_insert_list = smartlist_new();
  MOCK(router_get_by_extrainfo_digest, mock_get_by_ei_desc_digest);
  MOCK(extrainfo_insert, mock_ei_insert);

  smartlist_add_strdup(chunks, EX_EI_MINIMAL);
  smartlist_add_strdup(chunks, EX_EI_BAD_NICKNAME);
  smartlist_add_strdup(chunks, EX_EI_MAXIMAL);
  smartlist_add_strdup(chunks, EX_EI_BAD_PUBLISHED);
  smartlist_add_strdup(chunks, EX_EI_BAD_TOKENS);

  /* not ADDing MINIMIAL */
  ADD(EX_EI_MAXIMAL);
  ADD(EX_EI_BAD_NICKNAME);
  /* Not ADDing BAD_PUBLISHED */
  ADD(EX_EI_BAD_TOKENS);
  ADD(EX_EI_BAD_SIG2);

  list = smartlist_join_strings(chunks, "", 0, NULL);
  router_load_extrainfo_from_string(list, NULL, SAVED_IN_JOURNAL, wanted, 1);

  /* The "maximal" router was added. */
  /* "minimal" was also added, even though we didn't ask for it, since
   * that's what we do with extrainfos. */
  tt_int_op(smartlist_len(mock_ei_insert_list),OP_EQ,2);

  extrainfo_t *e = smartlist_get(mock_ei_insert_list, 0);
  test_memeq_hex(e->cache_info.signed_descriptor_digest,
                 "11E0EDF526950739F7769810FCACAB8C882FAEEE");

  e = smartlist_get(mock_ei_insert_list, 1);
  test_memeq_hex(e->cache_info.signed_descriptor_digest,
                 "47803B02A0E70E9E8BDA226CB1D74DE354D67DFF");
  tt_int_op(dls_minimal.n_download_failures, OP_EQ, 0);
  tt_int_op(dls_maximal.n_download_failures, OP_EQ, 0);

  /* "Bad nickname" and "Bad tokens" should have gotten marked
   * non-retriable. */
  tt_want_int_op(mock_get_by_ei_dd_calls, OP_EQ, 2);
  tt_want_int_op(mock_get_by_ei_dd_unrecognized, OP_EQ, 0);
  tt_int_op(sd_ei_bad_nickname.ei_dl_status.n_download_failures, OP_EQ, 255);
  tt_int_op(sd_ei_bad_tokens.ei_dl_status.n_download_failures, OP_EQ, 255);

  /* bad_ports is retriable -- because we didn't ask for it. */
  tt_int_op(dls_bad_ports.n_download_failures, OP_EQ, 0);

  /* Wanted still contains "BAD_SIG2" */
  tt_int_op(smartlist_len(wanted), OP_EQ, 1);
  tt_str_op(smartlist_get(wanted, 0), OP_EQ,
            "16D387D3A58F7DB3CF46638F8D0B90C45C7D769C");

#undef ADD

 done:
  tor_free(mem_op_hex_tmp);
  UNMOCK(router_get_by_extrainfo_digest);
  SMARTLIST_FOREACH(chunks, char *, cp, tor_free(cp));
  smartlist_free(chunks);
  SMARTLIST_FOREACH(wanted, char *, cp, tor_free(cp));
  smartlist_free(wanted);
  tor_free(list);
}

static void
test_dir_getinfo_extra(void *arg)
{
  int r;
  char *answer = NULL;
  const char *errmsg = NULL;

  (void)arg;
  MOCK(extrainfo_get_by_descriptor_digest, mock_ei_get_by_ei_digest);
  r = getinfo_helper_dir(NULL, "extra-info/digest/"
                         "11E0EDF526950739F7769810FCACAB8C882FAEEE", &answer,
                         &errmsg);
  tt_int_op(0, OP_EQ, r);
  tt_ptr_op(NULL, OP_EQ, errmsg);
  tt_str_op(answer, OP_EQ, EX_EI_MINIMAL);
  tor_free(answer);

  answer = NULL;
  r = getinfo_helper_dir(NULL, "extra-info/digest/"
                         "NOTAVALIDHEXSTRINGNOTAVALIDHEXSTRINGNOTA", &answer,
                         &errmsg);
  tt_int_op(0, OP_EQ, r);
  /* getinfo_helper_dir() should maybe return an error here but doesn't */
  tt_ptr_op(NULL, OP_EQ, errmsg);
  /* In any case, there should be no answer for an invalid hex string. */
  tt_ptr_op(NULL, OP_EQ, answer);

 done:
  UNMOCK(extrainfo_get_by_descriptor_digest);
}

static void
test_dir_versions(void *arg)
{
  tor_version_t ver1;

  /* Try out version parsing functionality */
  (void)arg;
  tt_int_op(0,OP_EQ, tor_version_parse("0.3.4pre2-cvs", &ver1));
  tt_int_op(0,OP_EQ, ver1.major);
  tt_int_op(3,OP_EQ, ver1.minor);
  tt_int_op(4,OP_EQ, ver1.micro);
  tt_int_op(VER_PRE,OP_EQ, ver1.status);
  tt_int_op(2,OP_EQ, ver1.patchlevel);
  tt_int_op(0,OP_EQ, tor_version_parse("0.3.4rc1", &ver1));
  tt_int_op(0,OP_EQ, ver1.major);
  tt_int_op(3,OP_EQ, ver1.minor);
  tt_int_op(4,OP_EQ, ver1.micro);
  tt_int_op(VER_RC,OP_EQ, ver1.status);
  tt_int_op(1,OP_EQ, ver1.patchlevel);
  tt_int_op(0,OP_EQ, tor_version_parse("1.3.4", &ver1));
  tt_int_op(1,OP_EQ, ver1.major);
  tt_int_op(3,OP_EQ, ver1.minor);
  tt_int_op(4,OP_EQ, ver1.micro);
  tt_int_op(VER_RELEASE,OP_EQ, ver1.status);
  tt_int_op(0,OP_EQ, ver1.patchlevel);
  tt_int_op(0,OP_EQ, tor_version_parse("1.3.4.999", &ver1));
  tt_int_op(1,OP_EQ, ver1.major);
  tt_int_op(3,OP_EQ, ver1.minor);
  tt_int_op(4,OP_EQ, ver1.micro);
  tt_int_op(VER_RELEASE,OP_EQ, ver1.status);
  tt_int_op(999,OP_EQ, ver1.patchlevel);
  tt_int_op(0,OP_EQ, tor_version_parse("0.1.2.4-alpha", &ver1));
  tt_int_op(0,OP_EQ, ver1.major);
  tt_int_op(1,OP_EQ, ver1.minor);
  tt_int_op(2,OP_EQ, ver1.micro);
  tt_int_op(4,OP_EQ, ver1.patchlevel);
  tt_int_op(VER_RELEASE,OP_EQ, ver1.status);
  tt_str_op("alpha",OP_EQ, ver1.status_tag);
  tt_int_op(0,OP_EQ, tor_version_parse("0.1.2.4", &ver1));
  tt_int_op(0,OP_EQ, ver1.major);
  tt_int_op(1,OP_EQ, ver1.minor);
  tt_int_op(2,OP_EQ, ver1.micro);
  tt_int_op(4,OP_EQ, ver1.patchlevel);
  tt_int_op(VER_RELEASE,OP_EQ, ver1.status);
  tt_str_op("",OP_EQ, ver1.status_tag);

  tt_int_op(0, OP_EQ, tor_version_parse("10.1", &ver1));
  tt_int_op(10, OP_EQ, ver1.major);
  tt_int_op(1, OP_EQ, ver1.minor);
  tt_int_op(0, OP_EQ, ver1.micro);
  tt_int_op(0, OP_EQ, ver1.patchlevel);
  tt_int_op(VER_RELEASE, OP_EQ, ver1.status);
  tt_str_op("", OP_EQ, ver1.status_tag);
  tt_int_op(0, OP_EQ, tor_version_parse("5.99.999", &ver1));
  tt_int_op(5, OP_EQ, ver1.major);
  tt_int_op(99, OP_EQ, ver1.minor);
  tt_int_op(999, OP_EQ, ver1.micro);
  tt_int_op(0, OP_EQ, ver1.patchlevel);
  tt_int_op(VER_RELEASE, OP_EQ, ver1.status);
  tt_str_op("", OP_EQ, ver1.status_tag);
  tt_int_op(0, OP_EQ, tor_version_parse("10.1-alpha", &ver1));
  tt_int_op(10, OP_EQ, ver1.major);
  tt_int_op(1, OP_EQ, ver1.minor);
  tt_int_op(0, OP_EQ, ver1.micro);
  tt_int_op(0, OP_EQ, ver1.patchlevel);
  tt_int_op(VER_RELEASE, OP_EQ, ver1.status);
  tt_str_op("alpha", OP_EQ, ver1.status_tag);
  /* Go through the full set of status tags */
  tt_int_op(0, OP_EQ, tor_version_parse("2.1.700-alpha", &ver1));
  tt_int_op(2, OP_EQ, ver1.major);
  tt_int_op(1, OP_EQ, ver1.minor);
  tt_int_op(700, OP_EQ, ver1.micro);
  tt_int_op(0, OP_EQ, ver1.patchlevel);
  tt_int_op(VER_RELEASE, OP_EQ, ver1.status);
  tt_str_op("alpha", OP_EQ, ver1.status_tag);
  tt_int_op(0, OP_EQ, tor_version_parse("1.6.8-alpha-dev", &ver1));
  tt_int_op(1, OP_EQ, ver1.major);
  tt_int_op(6, OP_EQ, ver1.minor);
  tt_int_op(8, OP_EQ, ver1.micro);
  tt_int_op(0, OP_EQ, ver1.patchlevel);
  tt_int_op(VER_RELEASE, OP_EQ, ver1.status);
  tt_str_op("alpha-dev", OP_EQ, ver1.status_tag);
  tt_int_op(0, OP_EQ, tor_version_parse("0.2.9.5-rc", &ver1));
  tt_int_op(0, OP_EQ, ver1.major);
  tt_int_op(2, OP_EQ, ver1.minor);
  tt_int_op(9, OP_EQ, ver1.micro);
  tt_int_op(5, OP_EQ, ver1.patchlevel);
  tt_int_op(VER_RELEASE, OP_EQ, ver1.status);
  tt_str_op("rc", OP_EQ, ver1.status_tag);
  tt_int_op(0, OP_EQ, tor_version_parse("0.2.9.6-rc-dev", &ver1));
  tt_int_op(0, OP_EQ, ver1.major);
  tt_int_op(2, OP_EQ, ver1.minor);
  tt_int_op(9, OP_EQ, ver1.micro);
  tt_int_op(6, OP_EQ, ver1.patchlevel);
  tt_int_op(VER_RELEASE, OP_EQ, ver1.status);
  tt_str_op("rc-dev", OP_EQ, ver1.status_tag);
  tt_int_op(0, OP_EQ, tor_version_parse("0.2.9.8", &ver1));
  tt_int_op(0, OP_EQ, ver1.major);
  tt_int_op(2, OP_EQ, ver1.minor);
  tt_int_op(9, OP_EQ, ver1.micro);
  tt_int_op(8, OP_EQ, ver1.patchlevel);
  tt_int_op(VER_RELEASE, OP_EQ, ver1.status);
  tt_str_op("", OP_EQ, ver1.status_tag);
  tt_int_op(0, OP_EQ, tor_version_parse("0.2.9.9-dev", &ver1));
  tt_int_op(0, OP_EQ, ver1.major);
  tt_int_op(2, OP_EQ, ver1.minor);
  tt_int_op(9, OP_EQ, ver1.micro);
  tt_int_op(9, OP_EQ, ver1.patchlevel);
  tt_int_op(VER_RELEASE, OP_EQ, ver1.status);
  tt_str_op("dev", OP_EQ, ver1.status_tag);
  /* In #21450, we fixed an inconsistency in parsing versions > INT32_MAX
   * between i386 and x86_64, as we used tor_parse_long, and then cast to int
   */
  tt_int_op(0, OP_EQ, tor_version_parse("0.2147483647.0", &ver1));
  tt_int_op(0, OP_EQ, ver1.major);
  tt_int_op(2147483647, OP_EQ, ver1.minor);
  tt_int_op(0, OP_EQ, ver1.micro);
  tt_int_op(0, OP_EQ, ver1.patchlevel);
  tt_int_op(VER_RELEASE, OP_EQ, ver1.status);
  tt_str_op("", OP_EQ, ver1.status_tag);
  tt_int_op(-1, OP_EQ, tor_version_parse("0.2147483648.0", &ver1));
  tt_int_op(-1, OP_EQ, tor_version_parse("0.4294967295.0", &ver1));
  /* In #21278, we reject negative version components */
  tt_int_op(-1, OP_EQ, tor_version_parse("0.-1.0", &ver1));
  tt_int_op(-1, OP_EQ, tor_version_parse("0.-2147483648.0", &ver1));
  tt_int_op(-1, OP_EQ, tor_version_parse("0.-4294967295.0", &ver1));
  /* In #21507, we reject version components with non-numeric prefixes */
  tt_int_op(-1, OP_EQ, tor_version_parse("0.-0.0", &ver1));
  tt_int_op(-1, OP_EQ, tor_version_parse("+1.0.0", &ver1));
  /* use the list in isspace() */
  tt_int_op(-1, OP_EQ, tor_version_parse("0.\t0.0", &ver1));
  tt_int_op(-1, OP_EQ, tor_version_parse("0.\n0.0", &ver1));
  tt_int_op(-1, OP_EQ, tor_version_parse("0.\v0.0", &ver1));
  tt_int_op(-1, OP_EQ, tor_version_parse("0.\f0.0", &ver1));
  tt_int_op(-1, OP_EQ, tor_version_parse("0.\r0.0", &ver1));
  tt_int_op(-1, OP_EQ, tor_version_parse("0. 0.0", &ver1));

#define tt_versionstatus_op(vs1, op, vs2)                               \
  tt_assert_test_type(vs1,vs2,#vs1" "#op" "#vs2,version_status_t,       \
                      (val1_ op val2_),"%d",TT_EXIT_TEST_FUNCTION)
#define test_v_i_o(val, ver, lst)                                       \
  tt_versionstatus_op(val, OP_EQ, tor_version_is_obsolete(ver, lst))

  /* make sure tor_version_is_obsolete() works */
  test_v_i_o(VS_OLD, "0.0.1", "Tor 0.0.2");
  test_v_i_o(VS_OLD, "0.0.1", "0.0.2, Tor 0.0.3");
  test_v_i_o(VS_OLD, "0.0.1", "0.0.2,Tor 0.0.3");
  test_v_i_o(VS_OLD, "0.0.1","0.0.3,BetterTor 0.0.1");
  test_v_i_o(VS_RECOMMENDED, "0.0.2", "Tor 0.0.2,Tor 0.0.3");
  test_v_i_o(VS_NEW_IN_SERIES, "0.0.2", "Tor 0.0.2pre1,Tor 0.0.3");
  test_v_i_o(VS_OLD, "0.0.2", "Tor 0.0.2.1,Tor 0.0.3");
  test_v_i_o(VS_NEW, "0.1.0", "Tor 0.0.2,Tor 0.0.3");
  test_v_i_o(VS_RECOMMENDED, "0.0.7rc2", "0.0.7,Tor 0.0.7rc2,Tor 0.0.8");
  test_v_i_o(VS_OLD, "0.0.5.0", "0.0.5.1-cvs");
  test_v_i_o(VS_NEW_IN_SERIES, "0.0.5.1-cvs", "0.0.5, 0.0.6");
  test_v_i_o(VS_NEW, "0.2.9.9-dev", "0.2.9.9");
  /* Not on list, but newer than any in same series. */
  test_v_i_o(VS_NEW_IN_SERIES, "0.1.0.3",
             "Tor 0.1.0.2,Tor 0.0.9.5,Tor 0.1.1.0");
  /* Series newer than any on list. */
  test_v_i_o(VS_NEW, "0.1.2.3", "Tor 0.1.0.2,Tor 0.0.9.5,Tor 0.1.1.0");
  /* Series older than any on list. */
  test_v_i_o(VS_OLD, "0.0.1.3", "Tor 0.1.0.2,Tor 0.0.9.5,Tor 0.1.1.0");
  /* Not on list, not newer than any on same series. */
  test_v_i_o(VS_UNRECOMMENDED, "0.1.0.1",
             "Tor 0.1.0.2,Tor 0.0.9.5,Tor 0.1.1.0");
  /* On list, not newer than any on same series. */
  test_v_i_o(VS_UNRECOMMENDED,
             "0.1.0.1", "Tor 0.1.0.2,Tor 0.0.9.5,Tor 0.1.1.0");
  tt_int_op(0,OP_EQ, tor_version_as_new_as("Tor 0.0.5", "0.0.9pre1-cvs"));
  tt_int_op(1,OP_EQ, tor_version_as_new_as(
          "Tor 0.0.8 on Darwin 64-121-192-100.c3-0."
          "sfpo-ubr1.sfrn-sfpo.ca.cable.rcn.com Power Macintosh",
          "0.0.8rc2"));
  tt_int_op(0,OP_EQ, tor_version_as_new_as(
          "Tor 0.0.8 on Darwin 64-121-192-100.c3-0."
          "sfpo-ubr1.sfrn-sfpo.ca.cable.rcn.com Power Macintosh", "0.0.8.2"));

  /* Now try svn revisions. */
  tt_int_op(1,OP_EQ, tor_version_as_new_as("Tor 0.2.1.0-dev (r100)",
                                   "Tor 0.2.1.0-dev (r99)"));
  tt_int_op(1,OP_EQ, tor_version_as_new_as(
                                   "Tor 0.2.1.0-dev (r100) on Banana Jr",
                                   "Tor 0.2.1.0-dev (r99) on Hal 9000"));
  tt_int_op(1,OP_EQ, tor_version_as_new_as("Tor 0.2.1.0-dev (r100)",
                                   "Tor 0.2.1.0-dev on Colossus"));
  tt_int_op(0,OP_EQ, tor_version_as_new_as("Tor 0.2.1.0-dev (r99)",
                                   "Tor 0.2.1.0-dev (r100)"));
  tt_int_op(0,OP_EQ, tor_version_as_new_as("Tor 0.2.1.0-dev (r99) on MCP",
                                   "Tor 0.2.1.0-dev (r100) on AM"));
  tt_int_op(0,OP_EQ, tor_version_as_new_as("Tor 0.2.1.0-dev",
                                   "Tor 0.2.1.0-dev (r99)"));
  tt_int_op(1,OP_EQ, tor_version_as_new_as("Tor 0.2.1.1",
                                   "Tor 0.2.1.0-dev (r99)"));
  /* And git revisions */
  tt_int_op(1,OP_EQ, tor_version_as_new_as(
                                        "Tor 0.2.9.9 (git-56788a2489127072)",
                                        "Tor 0.2.9.9 (git-56788a2489127072)"));
  /* a git revision is newer than no git revision */
  tt_int_op(1,OP_EQ, tor_version_as_new_as(
                                        "Tor 0.2.9.9 (git-56788a2489127072)",
                                        "Tor 0.2.9.9"));
  /* a longer git revision is newer than a shorter git revision
   * this should be true if they prefix-match, but if they don't, they are
   * incomparable, because hashes aren't ordered (but we compare their bytes
   * anyway) */
  tt_int_op(1,OP_EQ, tor_version_as_new_as(
                  "Tor 0.2.9.9 (git-56788a2489127072d513cf4baf35a8ff475f3c7b)",
                  "Tor 0.2.9.9 (git-56788a2489127072)"));
  tt_int_op(1,OP_EQ, tor_version_as_new_as(
                                        "Tor 0.2.9.9 (git-0102)",
                                        "Tor 0.2.9.9 (git-03)"));
  tt_int_op(1,OP_EQ, tor_version_as_new_as(
                                        "Tor 0.2.9.9 (git-0102)",
                                        "Tor 0.2.9.9 (git-00)"));
  tt_int_op(1,OP_EQ, tor_version_as_new_as(
                                           "Tor 0.2.9.9 (git-01)",
                                           "Tor 0.2.9.9 (git-00)"));
  tt_int_op(0,OP_EQ, tor_version_as_new_as(
                                           "Tor 0.2.9.9 (git-00)",
                                           "Tor 0.2.9.9 (git-01)"));
  /* In #21278, we compare without integer overflows.
   * But since #21450 limits version components to [0, INT32_MAX], it is no
   * longer possible to cause an integer overflow in tor_version_compare() */
  tt_int_op(0,OP_EQ, tor_version_as_new_as(
                                           "Tor 0.0.0.0",
                                           "Tor 2147483647.0.0.0"));
  tt_int_op(1,OP_EQ, tor_version_as_new_as(
                                           "Tor 2147483647.0.0.0",
                                           "Tor 0.0.0.0"));
  /* These versions used to cause an overflow, now they don't parse
   * (and authorities reject their descriptors), and log a BUG message */
  setup_full_capture_of_logs(LOG_WARN);
  tt_int_op(0,OP_EQ, tor_version_as_new_as(
                                           "Tor 0.0.0.0",
                                           "Tor 0.-2147483648.0.0"));
  expect_single_log_msg_containing("unparseable");
  mock_clean_saved_logs();
  tt_int_op(0,OP_EQ, tor_version_as_new_as(
                                           "Tor 0.2147483647.0.0",
                                           "Tor 0.-1.0.0"));
  expect_single_log_msg_containing("unparseable");
  mock_clean_saved_logs();
  tt_int_op(0,OP_EQ, tor_version_as_new_as(
                                           "Tor 0.2147483647.0.0",
                                           "Tor 0.-2147483648.0.0"));
  expect_single_log_msg_containing("unparseable");
  mock_clean_saved_logs();
  tt_int_op(1,OP_EQ, tor_version_as_new_as(
                                           "Tor 4294967295.0.0.0",
                                           "Tor 0.0.0.0"));
  expect_no_log_entry();
  tt_int_op(0,OP_EQ, tor_version_as_new_as(
                                           "Tor 0.4294967295.0.0",
                                           "Tor 0.-4294967295.0.0"));
  expect_single_log_msg_containing("unparseable");
  mock_clean_saved_logs();
  teardown_capture_of_logs();

  /* Now try git revisions */
  tt_int_op(0,OP_EQ, tor_version_parse("0.5.6.7 (git-ff00ff)", &ver1));
  tt_int_op(0,OP_EQ, ver1.major);
  tt_int_op(5,OP_EQ, ver1.minor);
  tt_int_op(6,OP_EQ, ver1.micro);
  tt_int_op(7,OP_EQ, ver1.patchlevel);
  tt_int_op(3,OP_EQ, ver1.git_tag_len);
  tt_mem_op(ver1.git_tag,OP_EQ, "\xff\x00\xff", 3);
  /* reject bad hex digits */
  tt_int_op(-1,OP_EQ, tor_version_parse("0.5.6.7 (git-ff00xx)", &ver1));
  /* reject odd hex digit count */
  tt_int_op(-1,OP_EQ, tor_version_parse("0.5.6.7 (git-ff00fff)", &ver1));
  /* ignore "git " */
  tt_int_op(0,OP_EQ, tor_version_parse("0.5.6.7 (git ff00fff)", &ver1));
  /* standard length is 16 hex digits */
  tt_int_op(0,OP_EQ, tor_version_parse("0.5.6.7 (git-0010203040506070)",
                                       &ver1));
  /* length limit is 40 hex digits */
  tt_int_op(0,OP_EQ, tor_version_parse(
                     "0.5.6.7 (git-000102030405060708090a0b0c0d0e0f10111213)",
                     &ver1));
  tt_int_op(-1,OP_EQ, tor_version_parse(
                    "0.5.6.7 (git-000102030405060708090a0b0c0d0e0f1011121314)",
                    &ver1));
 done:
  teardown_capture_of_logs();
}

/** Run unit tests for directory fp_pair functions. */
static void
test_dir_fp_pairs(void *arg)
{
  smartlist_t *sl = smartlist_new();
  fp_pair_t *pair;

  (void)arg;
  dir_split_resource_into_fingerprint_pairs(
       /* Two pairs, out of order, with one duplicate. */
       "73656372657420646174612E0000000000FFFFFF-"
       "557365204145532d32353620696e73746561642e+"
       "73656372657420646174612E0000000000FFFFFF-"
       "557365204145532d32353620696e73746561642e+"
       "48657861646563696d616c2069736e277420736f-"
       "676f6f6420666f7220686964696e6720796f7572.z", sl);

  tt_int_op(smartlist_len(sl),OP_EQ, 2);
  pair = smartlist_get(sl, 0);
  tt_mem_op(pair->first,OP_EQ,  "Hexadecimal isn't so", DIGEST_LEN);
  tt_mem_op(pair->second,OP_EQ, "good for hiding your", DIGEST_LEN);
  pair = smartlist_get(sl, 1);
  tt_mem_op(pair->first,OP_EQ,  "secret data.\0\0\0\0\0\xff\xff\xff",
            DIGEST_LEN);
  tt_mem_op(pair->second,OP_EQ, "Use AES-256 instead.", DIGEST_LEN);

 done:
  SMARTLIST_FOREACH(sl, fp_pair_t *, pair_to_free, tor_free(pair_to_free));
  smartlist_free(sl);
}

static void
test_dir_split_fps(void *testdata)
{
  smartlist_t *sl = smartlist_new();
  char *mem_op_hex_tmp = NULL;
  (void)testdata;

  /* Some example hex fingerprints and their base64 equivalents */
#define HEX1 "Fe0daff89127389bc67558691231234551193EEE"
#define HEX2 "Deadbeef99999991111119999911111111f00ba4"
#define HEX3 "b33ff00db33ff00db33ff00db33ff00db33ff00d"
#define HEX256_1 \
    "f3f3f3f3fbbbbf3f3f3f3fbbbf3f3f3f3fbbbbf3f3f3f3fbbbf3f3f3f3fbbbbf"
#define HEX256_2 \
    "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccCCc"
#define HEX256_3 \
    "0123456789ABCdef0123456789ABCdef0123456789ABCdef0123456789ABCdef"
#define B64_1 "/g2v+JEnOJvGdVhpEjEjRVEZPu4"
#define B64_2 "3q2+75mZmZERERmZmRERERHwC6Q"
#define B64_256_1 "8/Pz8/u7vz8/Pz+7vz8/Pz+7u/Pz8/P7u/Pz8/P7u78"
#define B64_256_2 "zMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMw"

  /* no flags set */
  dir_split_resource_into_fingerprints("A+C+B", sl, NULL, 0);
  tt_int_op(smartlist_len(sl), OP_EQ, 3);
  tt_str_op(smartlist_get(sl, 0), OP_EQ, "A");
  tt_str_op(smartlist_get(sl, 1), OP_EQ, "C");
  tt_str_op(smartlist_get(sl, 2), OP_EQ, "B");
  SMARTLIST_FOREACH(sl, char *, cp, tor_free(cp));
  smartlist_clear(sl);

  /* uniq strings. */
  dir_split_resource_into_fingerprints("A+C+B+A+B+B", sl, NULL, DSR_SORT_UNIQ);
  tt_int_op(smartlist_len(sl), OP_EQ, 3);
  tt_str_op(smartlist_get(sl, 0), OP_EQ, "A");
  tt_str_op(smartlist_get(sl, 1), OP_EQ, "B");
  tt_str_op(smartlist_get(sl, 2), OP_EQ, "C");
  SMARTLIST_FOREACH(sl, char *, cp, tor_free(cp));
  smartlist_clear(sl);

  /* Decode hex. */
  dir_split_resource_into_fingerprints(HEX1"+"HEX2, sl, NULL, DSR_HEX);
  tt_int_op(smartlist_len(sl), OP_EQ, 2);
  test_mem_op_hex(smartlist_get(sl, 0), OP_EQ, HEX1);
  test_mem_op_hex(smartlist_get(sl, 1), OP_EQ, HEX2);
  SMARTLIST_FOREACH(sl, char *, cp, tor_free(cp));
  smartlist_clear(sl);

  /* decode hex and drop weirdness. */
  dir_split_resource_into_fingerprints(HEX1"+bogus+"HEX2"+"HEX256_1,
                                       sl, NULL, DSR_HEX);
  tt_int_op(smartlist_len(sl), OP_EQ, 2);
  test_mem_op_hex(smartlist_get(sl, 0), OP_EQ, HEX1);
  test_mem_op_hex(smartlist_get(sl, 1), OP_EQ, HEX2);
  SMARTLIST_FOREACH(sl, char *, cp, tor_free(cp));
  smartlist_clear(sl);

  /* Decode long hex */
  dir_split_resource_into_fingerprints(HEX256_1"+"HEX256_2"+"HEX2"+"HEX256_3,
                                       sl, NULL, DSR_HEX|DSR_DIGEST256);
  tt_int_op(smartlist_len(sl), OP_EQ, 3);
  test_mem_op_hex(smartlist_get(sl, 0), OP_EQ, HEX256_1);
  test_mem_op_hex(smartlist_get(sl, 1), OP_EQ, HEX256_2);
  test_mem_op_hex(smartlist_get(sl, 2), OP_EQ, HEX256_3);
  SMARTLIST_FOREACH(sl, char *, cp, tor_free(cp));
  smartlist_clear(sl);

  /* Decode hex and sort. */
  dir_split_resource_into_fingerprints(HEX1"+"HEX2"+"HEX3"+"HEX2,
                                       sl, NULL, DSR_HEX|DSR_SORT_UNIQ);
  tt_int_op(smartlist_len(sl), OP_EQ, 3);
  test_mem_op_hex(smartlist_get(sl, 0), OP_EQ, HEX3);
  test_mem_op_hex(smartlist_get(sl, 1), OP_EQ, HEX2);
  test_mem_op_hex(smartlist_get(sl, 2), OP_EQ, HEX1);
  SMARTLIST_FOREACH(sl, char *, cp, tor_free(cp));
  smartlist_clear(sl);

  /* Decode long hex and sort */
  dir_split_resource_into_fingerprints(HEX256_1"+"HEX256_2"+"HEX256_3
                                       "+"HEX256_1,
                                       sl, NULL,
                                       DSR_HEX|DSR_DIGEST256|DSR_SORT_UNIQ);
  tt_int_op(smartlist_len(sl), OP_EQ, 3);
  test_mem_op_hex(smartlist_get(sl, 0), OP_EQ, HEX256_3);
  test_mem_op_hex(smartlist_get(sl, 1), OP_EQ, HEX256_2);
  test_mem_op_hex(smartlist_get(sl, 2), OP_EQ, HEX256_1);
  SMARTLIST_FOREACH(sl, char *, cp, tor_free(cp));
  smartlist_clear(sl);

  /* Decode base64 */
  dir_split_resource_into_fingerprints(B64_1"-"B64_2, sl, NULL, DSR_BASE64);
  tt_int_op(smartlist_len(sl), OP_EQ, 2);
  test_mem_op_hex(smartlist_get(sl, 0), OP_EQ, HEX1);
  test_mem_op_hex(smartlist_get(sl, 1), OP_EQ, HEX2);
  SMARTLIST_FOREACH(sl, char *, cp, tor_free(cp));
  smartlist_clear(sl);

  /* Decode long base64 */
  dir_split_resource_into_fingerprints(B64_256_1"-"B64_256_2,
                                       sl, NULL, DSR_BASE64|DSR_DIGEST256);
  tt_int_op(smartlist_len(sl), OP_EQ, 2);
  test_mem_op_hex(smartlist_get(sl, 0), OP_EQ, HEX256_1);
  test_mem_op_hex(smartlist_get(sl, 1), OP_EQ, HEX256_2);
  SMARTLIST_FOREACH(sl, char *, cp, tor_free(cp));
  smartlist_clear(sl);

  dir_split_resource_into_fingerprints(B64_256_1,
                                       sl, NULL, DSR_BASE64|DSR_DIGEST256);
  tt_int_op(smartlist_len(sl), OP_EQ, 1);
  test_mem_op_hex(smartlist_get(sl, 0), OP_EQ, HEX256_1);
  SMARTLIST_FOREACH(sl, char *, cp, tor_free(cp));
  smartlist_clear(sl);

 done:
  SMARTLIST_FOREACH(sl, char *, cp, tor_free(cp));
  smartlist_free(sl);
  tor_free(mem_op_hex_tmp);
}

static void
test_dir_measured_bw_kb(void *arg)
{
  measured_bw_line_t mbwl;
  int i;
  const char *lines_pass[] = {
    "node_id=$557365204145532d32353620696e73746561642e bw=1024\n",
    "node_id=$557365204145532d32353620696e73746561642e\t  bw=1024 \n",
    " node_id=$557365204145532d32353620696e73746561642e  bw=1024\n",
    "\tnoise\tnode_id=$557365204145532d32353620696e73746561642e  "
                "bw=1024 junk=007\n",
    "misc=junk node_id=$557365204145532d32353620696e73746561642e  "
                "bw=1024 junk=007\n",
    /* check whether node_id can be at the end */
    "bw=1024 node_id=$557365204145532d32353620696e73746561642e\n",
    /* check whether node_id can be at the end and bw has something in front*/
    "foo=bar bw=1024 node_id=$557365204145532d32353620696e73746561642e\n",
    /* check whether node_id can be at the end and something in the
     * in the middle of bw and node_id */
    "bw=1024 foo=bar node_id=$557365204145532d32353620696e73746561642e\n",

    /* Test that a line with vote=1 will pass. */
    "node_id=$557365204145532d32353620696e73746561642e bw=1024 vote=1\n",
    /* Test that a line with unmeasured=1 will pass. */
    "node_id=$557365204145532d32353620696e73746561642e bw=1024 unmeasured=1\n",
    /* Test that a line with vote=1 and unmeasured=1 will pass. */
    "node_id=$557365204145532d32353620696e73746561642e bw=1024 vote=1"
    "unmeasured=1\n",
    /* Test that a line with unmeasured=0 will pass. */
    "node_id=$557365204145532d32353620696e73746561642e bw=1024 unmeasured=0\n",
    /* Test that a line with vote=1 and unmeasured=0 will pass. */
    "node_id=$557365204145532d32353620696e73746561642e bw=1024 vote=1"
    "unmeasured=0\n",
    "end"
  };
  const char *lines_fail[] = {
    /* Test possible python stupidity on input */
    "node_id=None bw=1024\n",
    "node_id=$None bw=1024\n",
    "node_id=$557365204145532d32353620696e73746561642e bw=None\n",
    "node_id=$557365204145532d32353620696e73746561642e bw=1024.0\n",
    "node_id=$557365204145532d32353620696e73746561642e bw=.1024\n",
    "node_id=$557365204145532d32353620696e73746561642e bw=1.024\n",
    "node_id=$557365204145532d32353620696e73746561642e bw=1024 bw=0\n",
    "node_id=$557365204145532d32353620696e73746561642e bw=1024 bw=None\n",
    "node_id=$557365204145532d32353620696e73746561642e bw=-1024\n",
    /* Test incomplete writes due to race conditions, partial copies, etc */
    "node_i",
    "node_i\n",
    "node_id=",
    "node_id=\n",
    "node_id=$557365204145532d32353620696e73746561642e bw=",
    "node_id=$557365204145532d32353620696e73746561642e bw=1024",
    "node_id=$557365204145532d32353620696e73746561642e bw=\n",
    "node_id=$557365204145532d32353620696e7374",
    "node_id=$557365204145532d32353620696e7374\n",
    "",
    "\n",
    " \n ",
    " \n\n",
    /* Test assorted noise */
    " node_id= ",
    "node_id==$557365204145532d32353620696e73746561642e bw==1024\n",
    "node_id=$55736520414552d32353620696e73746561642e bw=1024\n",
    "node_id=557365204145532d32353620696e73746561642e bw=1024\n",
    "node_id= $557365204145532d32353620696e73746561642e bw=0.23\n",

    /* Test that a line with vote=0 will fail too, so that it is ignored. */
    "node_id=$557365204145532d32353620696e73746561642e bw=1024 vote=0\n",
    /* Test that a line with vote=0 will fail even if unmeasured=0. */
    "node_id=$557365204145532d32353620696e73746561642e bw=1024 vote=0 "
    "unmeasured=0\n",
    "end"
  };

  (void)arg;
  for (i = 0; strcmp(lines_fail[i], "end"); i++) {
    //fprintf(stderr, "Testing: %s\n", lines_fail[i]);
    /* Testing only with line_is_after_headers = 1. Tests with
     * line_is_after_headers = 0 in
     * test_dir_measured_bw_kb_line_is_after_headers */
    tt_assert(measured_bw_line_parse(&mbwl, lines_fail[i], 1) == -1);
  }

  for (i = 0; strcmp(lines_pass[i], "end"); i++) {
    //fprintf(stderr, "Testing: %s %d\n", lines_pass[i], TOR_ISSPACE('\n'));
    /* Testing only with line_is_after_headers = 1. Tests with
     * line_is_after_headers = 0 in
     * test_dir_measured_bw_kb_line_is_after_headers */
    tt_assert(measured_bw_line_parse(&mbwl, lines_pass[i], 1) == 0);
    tt_assert(mbwl.bw_kb == 1024);
    tt_assert(strcmp(mbwl.node_hex,
                "557365204145532d32353620696e73746561642e") == 0);
  }

 done:
  return;
}

/* Unit tests for measured_bw_line_parse using line_is_after_headers flag.
 * When the end of the header is detected (a first complete bw line is parsed),
 * incomplete lines fail and give warnings, but do not give warnings if
 * the header is not ended, allowing to ignore additional header lines. */
static void
test_dir_measured_bw_kb_line_is_after_headers(void *arg)
{
  (void)arg;
  measured_bw_line_t mbwl;
  const char *line_pass = \
    "node_id=$557365204145532d32353620696e73746561642e bw=1024\n";
  int i;
  const char *lines_fail[] = {
    "node_id=$557365204145532d32353620696e73746561642e \n",
    "bw=1024\n",
    "rtt=300\n",
    "end"
  };

  setup_capture_of_logs(LOG_DEBUG);

  /* Test bw lines when header has ended */
  for (i = 0; strcmp(lines_fail[i], "end"); i++) {
    tt_assert(measured_bw_line_parse(&mbwl, lines_fail[i], 1) == -1);
    expect_log_msg_containing("Incomplete line in bandwidth file:");
    mock_clean_saved_logs();
  }

  tt_assert(measured_bw_line_parse(&mbwl, line_pass, 1) == 0);

  /* Test bw lines when header has not ended */
  for (i = 0; strcmp(lines_fail[i], "end"); i++) {
    tt_assert(measured_bw_line_parse(&mbwl, lines_fail[i], 0) == -1);
    expect_log_msg_containing("Missing bw or node_id in bandwidth file line:");
    mock_clean_saved_logs();
  }

  tt_assert(measured_bw_line_parse(&mbwl, line_pass, 0) == 0);

 done:
  teardown_capture_of_logs();
}

/* Test dirserv_read_measured_bandwidths with headers and complete files. */
static void
test_dir_dirserv_read_measured_bandwidths(void *arg)
{
  (void)arg;
  char *content = NULL;
  time_t timestamp = time(NULL);
  char *fname = tor_strdup(get_fname("V3BandwidthsFile"));
  smartlist_t *bw_file_headers = smartlist_new();
  /* bw file strings in vote */
  char *bw_file_headers_str = NULL;
  char *bw_file_headers_str_v100 = NULL;
  char *bw_file_headers_str_v110 = NULL;
  char *bw_file_headers_str_bad = NULL;
  char *bw_file_headers_str_extra = NULL;
  char bw_file_headers_str_long[MAX_BW_FILE_HEADER_COUNT_IN_VOTE * 8 + 1] = "";
  /* string header lines in bw file */
  char *header_lines_v100 = NULL;
  char *header_lines_v110_no_terminator = NULL;
  char *header_lines_v110 = NULL;
  char header_lines_long[MAX_BW_FILE_HEADER_COUNT_IN_VOTE * 8 + 1] = "";
  int i;
  const char *header_lines_v110_no_terminator_no_timestamp =
    "version=1.1.0\n"
    "software=sbws\n"
    "software_version=0.1.0\n"
    "earliest_bandwidth=2018-05-08T16:13:26\n"
    "file_created=2018-04-16T21:49:18\n"
    "generator_started=2018-05-08T16:13:25\n"
    "latest_bandwidth=2018-04-16T20:49:18\n";
  const char *bw_file_headers_str_v110_no_timestamp =
    "version=1.1.0 software=sbws "
    "software_version=0.1.0 "
    "earliest_bandwidth=2018-05-08T16:13:26 "
    "file_created=2018-04-16T21:49:18 "
    "generator_started=2018-05-08T16:13:25 "
    "latest_bandwidth=2018-04-16T20:49:18";
  const char *relay_lines_v100 =
    "node_id=$557365204145532d32353620696e73746561642e bw=1024 "
    "nick=Test measured_at=1523911725 updated_at=1523911725 "
    "pid_error=4.11374090719 pid_error_sum=4.11374090719 "
    "pid_bw=57136645 pid_delta=2.12168374577 circ_fail=0.2 "
    "scanner=/filepath\n";
  const char *relay_lines_v110 =
    "node_id=$68A483E05A2ABDCA6DA5A3EF8DB5177638A27F80 "
    "master_key_ed25519=YaqV4vbvPYKucElk297eVdNArDz9HtIwUoIeo0+cVIpQ "
    "bw=760 nick=Test rtt=380 time=2018-05-08T16:13:26\n";
  const char *relay_lines_bad =
    "node_id=$68A483E05A2ABDCA6DA5A3EF8DB5177638A\n";

  tor_asprintf(&header_lines_v100, "%ld\n", (long)timestamp);
  tor_asprintf(&header_lines_v110_no_terminator, "%ld\n%s", (long)timestamp,
               header_lines_v110_no_terminator_no_timestamp);
  tor_asprintf(&header_lines_v110, "%s%s",
               header_lines_v110_no_terminator, BW_FILE_HEADERS_TERMINATOR);

  tor_asprintf(&bw_file_headers_str_v100, "timestamp=%ld",(long)timestamp);
  tor_asprintf(&bw_file_headers_str_v110, "timestamp=%ld %s",
               (long)timestamp, bw_file_headers_str_v110_no_timestamp);
  tor_asprintf(&bw_file_headers_str_bad, "%s "
               "node_id=$68A483E05A2ABDCA6DA5A3EF8DB5177638A",
               bw_file_headers_str_v110);

  for (i=0; i<MAX_BW_FILE_HEADER_COUNT_IN_VOTE; i++) {
    strlcat(header_lines_long, "foo=bar\n",
            sizeof(header_lines_long));
  }
  /* 8 is the number of v110 lines in header_lines_v110 */
  for (i=0; i<MAX_BW_FILE_HEADER_COUNT_IN_VOTE - 8 - 1; i++) {
    strlcat(bw_file_headers_str_long, "foo=bar ",
            sizeof(bw_file_headers_str_long));
  }
  strlcat(bw_file_headers_str_long, "foo=bar",
          sizeof(bw_file_headers_str_long));
  tor_asprintf(&bw_file_headers_str_extra,
               "%s %s",
               bw_file_headers_str_v110,
               bw_file_headers_str_long);

  /* Test an empty bandwidth file. bw_file_headers will be empty string */
  write_str_to_file(fname, "", 0);
  setup_capture_of_logs(LOG_WARN);
  tt_int_op(-1, OP_EQ, dirserv_read_measured_bandwidths(fname, NULL,
                                                        bw_file_headers,
                                                        NULL));
  expect_log_msg("Empty bandwidth file\n");
  teardown_capture_of_logs();
  bw_file_headers_str = smartlist_join_strings(bw_file_headers, " ", 0, NULL);
  tt_str_op("", OP_EQ, bw_file_headers_str);
  SMARTLIST_FOREACH(bw_file_headers, char *, c, tor_free(c));
  smartlist_free(bw_file_headers);
  tor_free(bw_file_headers_str);

  /* Test bandwidth file with only timestamp.
   * bw_file_headers will be empty string */
  bw_file_headers = smartlist_new();
  tor_asprintf(&content, "%ld", (long)timestamp);
  write_str_to_file(fname, content, 0);
  tor_free(content);
  tt_int_op(-1, OP_EQ, dirserv_read_measured_bandwidths(fname, NULL,
                                                        bw_file_headers,
                                                        NULL));

  bw_file_headers_str = smartlist_join_strings(bw_file_headers, " ", 0, NULL);
  tt_str_op("", OP_EQ, bw_file_headers_str);
  SMARTLIST_FOREACH(bw_file_headers, char *, c, tor_free(c));
  smartlist_free(bw_file_headers);
  tor_free(bw_file_headers_str);

  /* Test v1.0.0 bandwidth file headers */
  write_str_to_file(fname, header_lines_v100, 0);
  bw_file_headers = smartlist_new();
  tt_int_op(0, OP_EQ, dirserv_read_measured_bandwidths(fname, NULL,
                                                       bw_file_headers,
                                                       NULL));

  bw_file_headers_str = smartlist_join_strings(bw_file_headers, " ", 0, NULL);
  tt_str_op(bw_file_headers_str_v100, OP_EQ, bw_file_headers_str);
  SMARTLIST_FOREACH(bw_file_headers, char *, c, tor_free(c));
  smartlist_free(bw_file_headers);
  tor_free(bw_file_headers_str);

  /* Test v1.0.0 complete bandwidth file */
  bw_file_headers = smartlist_new();
  tor_asprintf(&content, "%s%s", header_lines_v100, relay_lines_v100);
  write_str_to_file(fname, content, 0);
  tor_free(content);
  tt_int_op(0, OP_EQ, dirserv_read_measured_bandwidths(fname, NULL,
                                                       bw_file_headers,
                                                       NULL));

  bw_file_headers_str = smartlist_join_strings(bw_file_headers, " ", 0, NULL);
  tt_str_op(bw_file_headers_str_v100, OP_EQ, bw_file_headers_str);
  SMARTLIST_FOREACH(bw_file_headers, char *, c, tor_free(c));
  smartlist_free(bw_file_headers);
  tor_free(bw_file_headers_str);

  /* Test v1.0.0 complete bandwidth file with NULL bw_file_headers. */
  tor_asprintf(&content, "%s%s", header_lines_v100, relay_lines_v100);
  write_str_to_file(fname, content, 0);
  tor_free(content);
  tt_int_op(0, OP_EQ, dirserv_read_measured_bandwidths(fname, NULL, NULL,
                                                       NULL));

  /* Test bandwidth file including v1.1.0 bandwidth headers and
   * v1.0.0 relay lines. bw_file_headers will contain the v1.1.0 headers. */
  bw_file_headers = smartlist_new();
  tor_asprintf(&content, "%s%s%s", header_lines_v100, header_lines_v110,
               relay_lines_v100);
  write_str_to_file(fname, content, 0);
  tor_free(content);
  tt_int_op(0, OP_EQ, dirserv_read_measured_bandwidths(fname, NULL,
                                                       bw_file_headers,
                                                       NULL));

  bw_file_headers_str = smartlist_join_strings(bw_file_headers, " ", 0, NULL);
  tt_str_op(bw_file_headers_str_v110, OP_EQ, bw_file_headers_str);
  SMARTLIST_FOREACH(bw_file_headers, char *, c, tor_free(c));
  smartlist_free(bw_file_headers);
  tor_free(bw_file_headers_str);

  /* Test v1.0.0 complete bandwidth file with v1.1.0 headers at the end.
   * bw_file_headers will contain only v1.0.0 headers and the additional
   * headers will be interpreted as malformed relay lines. */
  bw_file_headers = smartlist_new();
  tor_asprintf(&content, "%s%s%s", header_lines_v100, relay_lines_v100,
               header_lines_v110);
  write_str_to_file(fname, content, 0);
  tor_free(content);
  tt_int_op(0, OP_EQ, dirserv_read_measured_bandwidths(fname, NULL,
                                                       bw_file_headers,
                                                       NULL));

  bw_file_headers_str = smartlist_join_strings(bw_file_headers, " ", 0, NULL);
  tt_str_op(bw_file_headers_str_v100, OP_EQ, bw_file_headers_str);
  SMARTLIST_FOREACH(bw_file_headers, char *, c, tor_free(c));
  smartlist_free(bw_file_headers);
  tor_free(bw_file_headers_str);

  /* Test v1.0.0 complete bandwidth file, the v1.1.0 headers and more relay
   * lines. bw_file_headers will contain only v1.0.0 headers, the additional
   * headers will be interpreted as malformed relay lines and the last relay
   * lines will be correctly interpreted as relay lines. */
  bw_file_headers = smartlist_new();
  tor_asprintf(&content, "%s%s%s%s", header_lines_v100, relay_lines_v100,
               header_lines_v110, relay_lines_v100);
  write_str_to_file(fname, content, 0);
  tor_free(content);
  tt_int_op(0, OP_EQ, dirserv_read_measured_bandwidths(fname, NULL,
                                                       bw_file_headers,
                                                       NULL));

  bw_file_headers_str = smartlist_join_strings(bw_file_headers, " ", 0, NULL);
  tt_str_op(bw_file_headers_str_v100, OP_EQ, bw_file_headers_str);
  SMARTLIST_FOREACH(bw_file_headers, char *, c, tor_free(c));
  smartlist_free(bw_file_headers);
  tor_free(bw_file_headers_str);

  /* Test v1.1.0 bandwidth headers without terminator */
  bw_file_headers = smartlist_new();
  write_str_to_file(fname, header_lines_v110_no_terminator, 0);
  tt_int_op(0, OP_EQ, dirserv_read_measured_bandwidths(fname, NULL,
                                                       bw_file_headers,
                                                       NULL));

  bw_file_headers_str = smartlist_join_strings(bw_file_headers, " ", 0, NULL);
  tt_str_op(bw_file_headers_str_v110, OP_EQ, bw_file_headers_str);
  SMARTLIST_FOREACH(bw_file_headers, char *, c, tor_free(c));
  smartlist_free(bw_file_headers);
  tor_free(bw_file_headers_str);

  /* Test v1.1.0 bandwidth headers with terminator */
  bw_file_headers = smartlist_new();
  write_str_to_file(fname, header_lines_v110, 0);
  tt_int_op(0, OP_EQ, dirserv_read_measured_bandwidths(fname, NULL,
                                                       bw_file_headers,
                                                       NULL));

  bw_file_headers_str = smartlist_join_strings(bw_file_headers, " ", 0, NULL);
  tt_str_op(bw_file_headers_str_v110, OP_EQ, bw_file_headers_str);
  SMARTLIST_FOREACH(bw_file_headers, char *, c, tor_free(c));
  smartlist_free(bw_file_headers);
  tor_free(bw_file_headers_str);

  /* Test v1.1.0 bandwidth file without terminator, then relay lines.
   * bw_file_headers will contain the v1.1.0 headers. */
  bw_file_headers = smartlist_new();
  tor_asprintf(&content, "%s%s",
               header_lines_v110_no_terminator, relay_lines_v110);
  write_str_to_file(fname, content, 0);
  tor_free(content);
  tt_int_op(0, OP_EQ, dirserv_read_measured_bandwidths(fname, NULL,
                                                       bw_file_headers,
                                                       NULL));

  bw_file_headers_str = smartlist_join_strings(bw_file_headers, " ", 0, NULL);
  tt_str_op(bw_file_headers_str_v110, OP_EQ, bw_file_headers_str);
  SMARTLIST_FOREACH(bw_file_headers, char *, c, tor_free(c));
  smartlist_free(bw_file_headers);
  tor_free(bw_file_headers_str);

  /* Test v1.1.0 bandwidth headers with terminator, then relay lines
   * bw_file_headers will contain the v1.1.0 headers. */
  bw_file_headers = smartlist_new();
  tor_asprintf(&content, "%s%s",
               header_lines_v110, relay_lines_v110);
  write_str_to_file(fname, content, 0);
  tor_free(content);
  tt_int_op(0, OP_EQ, dirserv_read_measured_bandwidths(fname, NULL,
                                                       bw_file_headers,
                                                       NULL));

  bw_file_headers_str = smartlist_join_strings(bw_file_headers, " ", 0, NULL);
  tt_str_op(bw_file_headers_str_v110, OP_EQ, bw_file_headers_str);
  SMARTLIST_FOREACH(bw_file_headers, char *, c, tor_free(c));
  smartlist_free(bw_file_headers);
  tor_free(bw_file_headers_str);

  /* Test v1.1.0 bandwidth headers with terminator, then bad relay lines,
   * then terminator, then relay_lines_bad.
   * bw_file_headers will contain the v1.1.0 headers. */
  bw_file_headers = smartlist_new();
  tor_asprintf(&content, "%s%s%s%s", header_lines_v110, relay_lines_bad,
               BW_FILE_HEADERS_TERMINATOR, relay_lines_bad);
  write_str_to_file(fname, content, 0);
  tor_free(content);
  tt_int_op(0, OP_EQ, dirserv_read_measured_bandwidths(fname, NULL,
                                                       bw_file_headers,
                                                       NULL));

  bw_file_headers_str = smartlist_join_strings(bw_file_headers, " ", 0, NULL);
  tt_str_op(bw_file_headers_str_v110, OP_EQ, bw_file_headers_str);
  SMARTLIST_FOREACH(bw_file_headers, char *, c, tor_free(c));
  smartlist_free(bw_file_headers);
  tor_free(bw_file_headers_str);

  /* Test v1.1.0 bandwidth headers without terminator, then bad relay lines,
   * then relay lines. bw_file_headers will contain the v1.1.0 headers and
   * the bad relay lines. */
  bw_file_headers = smartlist_new();
  tor_asprintf(&content, "%s%s%s",
               header_lines_v110_no_terminator, relay_lines_bad,
               relay_lines_v110);
  write_str_to_file(fname, content, 0);
  tor_free(content);
  tt_int_op(0, OP_EQ, dirserv_read_measured_bandwidths(fname, NULL,
                                                       bw_file_headers,
                                                       NULL));

  bw_file_headers_str = smartlist_join_strings(bw_file_headers, " ", 0, NULL);
  tt_str_op(bw_file_headers_str_bad, OP_EQ, bw_file_headers_str);
  SMARTLIST_FOREACH(bw_file_headers, char *, c, tor_free(c));
  smartlist_free(bw_file_headers);
  tor_free(bw_file_headers_str);

  /* Test v1.1.0 bandwidth headers without terminator,
   * then many bad relay lines, then relay lines.
   * bw_file_headers will contain the v1.1.0 headers and the bad relay lines
   * to a maximum of MAX_BW_FILE_HEADER_COUNT_IN_VOTE header lines. */
  bw_file_headers = smartlist_new();
  tor_asprintf(&content, "%s%s%s",
               header_lines_v110_no_terminator, header_lines_long,
               relay_lines_v110);
  write_str_to_file(fname, content, 0);
  tor_free(content);
  tt_int_op(0, OP_EQ, dirserv_read_measured_bandwidths(fname, NULL,
                                                       bw_file_headers,
                                                       NULL));

  tt_int_op(MAX_BW_FILE_HEADER_COUNT_IN_VOTE, OP_EQ,
            smartlist_len(bw_file_headers));
  bw_file_headers_str = smartlist_join_strings(bw_file_headers, " ", 0, NULL);
  tt_str_op(bw_file_headers_str_extra, OP_EQ, bw_file_headers_str);
  SMARTLIST_FOREACH(bw_file_headers, char *, c, tor_free(c));
  smartlist_free(bw_file_headers);
  tor_free(bw_file_headers_str);

  /* Test v1.1.0 bandwidth headers without terminator,
   * then many bad relay lines, then relay lines.
   * bw_file_headers will contain the v1.1.0 headers and the bad relay lines.
   * Force bw_file_headers to have more than MAX_BW_FILE_HEADER_COUNT_IN_VOTE
   * This test is needed while there is not dirvote test. */
  bw_file_headers = smartlist_new();
  tor_asprintf(&content, "%s%s%s",
               header_lines_v110_no_terminator, header_lines_long,
               relay_lines_v110);
  write_str_to_file(fname, content, 0);
  tor_free(content);
  tt_int_op(0, OP_EQ, dirserv_read_measured_bandwidths(fname, NULL,
                                                       bw_file_headers,
                                                       NULL));

  tt_int_op(MAX_BW_FILE_HEADER_COUNT_IN_VOTE, OP_EQ,
            smartlist_len(bw_file_headers));
  /* force bw_file_headers to be bigger than
   * MAX_BW_FILE_HEADER_COUNT_IN_VOTE */
  char line[8] = "foo=bar\0";
  smartlist_add_strdup(bw_file_headers, line);
  tt_int_op(MAX_BW_FILE_HEADER_COUNT_IN_VOTE, OP_LT,
            smartlist_len(bw_file_headers));
  SMARTLIST_FOREACH(bw_file_headers, char *, c, tor_free(c));
  smartlist_free(bw_file_headers);
  tor_free(bw_file_headers_str);

  /* Test v1.x.x bandwidth line with vote=0.
   * It will be ignored it and logged it at debug level. */
  const char *relay_lines_ignore =
    "node_id=$68A483E05A2ABDCA6DA5A3EF8DB5177638A27F80 bw=1024 vote=0\n"
    "node_id=$68A483E05A2ABDCA6DA5A3EF8DB5177638A27F80 bw=1024 vote=0"
    "unmeasured=1\n"
    "node_id=$68A483E05A2ABDCA6DA5A3EF8DB5177638A27F80 bw=1024 vote=0"
    "unmeasured=0\n";

  /* Create the bandwidth file */
  tor_asprintf(&content, "%ld\n%s", (long)timestamp, relay_lines_ignore);
  write_str_to_file(fname, content, 0);
  tor_free(content);

  /* Read the bandwidth file */
  setup_full_capture_of_logs(LOG_DEBUG);
  tt_int_op(0, OP_EQ, dirserv_read_measured_bandwidths(fname, NULL, NULL,
                                                       NULL));
  expect_log_msg_containing("Ignoring bandwidth file line");
  teardown_capture_of_logs();

  /* Test v1.x.x bandwidth line with "vote=1" or "unmeasured=1" or
   * "unmeasured=0".
   * They will not be ignored. */
  /* Create the bandwidth file */
  const char *relay_lines_vote =
    "node_id=$68A483E05A2ABDCA6DA5A3EF8DB5177638A27F80 bw=1024 vote=1\n"
    "node_id=$68A483E05A2ABDCA6DA5A3EF8DB5177638A27F80 bw=1024 unmeasured=0\n"
    "node_id=$68A483E05A2ABDCA6DA5A3EF8DB5177638A27F80 bw=1024 unmeasured=1\n";
  tor_asprintf(&content, "%ld\n%s", (long)timestamp, relay_lines_vote);
  write_str_to_file(fname, content, 0);
  tor_free(content);

  /* Read the bandwidth file */
  setup_full_capture_of_logs(LOG_DEBUG);
  tt_int_op(0, OP_EQ, dirserv_read_measured_bandwidths(fname, NULL, NULL,
                                                       NULL));
  expect_log_msg_not_containing("Ignoring bandwidth file line");
  teardown_capture_of_logs();

 done:
  unlink(fname);
  tor_free(fname);
  tor_free(header_lines_v100);
  tor_free(header_lines_v110_no_terminator);
  tor_free(header_lines_v110);
  tor_free(bw_file_headers_str_v100);
  tor_free(bw_file_headers_str_v110);
  tor_free(bw_file_headers_str_bad);
  tor_free(bw_file_headers_str_extra);
}

#define MBWC_INIT_TIME 1000

/** Do the measured bandwidth cache unit test */
static void
test_dir_measured_bw_kb_cache(void *arg)
{
  /* Initial fake time_t for testing */
  time_t curr = MBWC_INIT_TIME;
  /* Some measured_bw_line_ts */
  measured_bw_line_t mbwl[3];
  /* For receiving output on cache queries */
  long bw;
  time_t as_of;

  /* First, clear the cache and assert that it's empty */
  (void)arg;
  dirserv_clear_measured_bw_cache();
  tt_int_op(dirserv_get_measured_bw_cache_size(),OP_EQ, 0);
  /*
   * Set up test mbwls; none of the dirserv_cache_*() functions care about
   * the node_hex field.
   */
  memset(mbwl[0].node_id, 0x01, DIGEST_LEN);
  mbwl[0].bw_kb = 20;
  memset(mbwl[1].node_id, 0x02, DIGEST_LEN);
  mbwl[1].bw_kb = 40;
  memset(mbwl[2].node_id, 0x03, DIGEST_LEN);
  mbwl[2].bw_kb = 80;
  /* Try caching something */
  dirserv_cache_measured_bw(&(mbwl[0]), curr);
  tt_int_op(dirserv_get_measured_bw_cache_size(),OP_EQ, 1);
  /* Okay, let's see if we can retrieve it */
  tt_assert(dirserv_query_measured_bw_cache_kb(mbwl[0].node_id,&bw, &as_of));
  tt_int_op(bw,OP_EQ, 20);
  tt_int_op(as_of,OP_EQ, MBWC_INIT_TIME);
  /* Try retrieving it without some outputs */
  tt_assert(dirserv_query_measured_bw_cache_kb(mbwl[0].node_id,NULL, NULL));
  tt_assert(dirserv_query_measured_bw_cache_kb(mbwl[0].node_id,&bw, NULL));
  tt_int_op(bw,OP_EQ, 20);
  tt_assert(dirserv_query_measured_bw_cache_kb(mbwl[0].node_id,NULL,&as_of));
  tt_int_op(as_of,OP_EQ, MBWC_INIT_TIME);
  /* Now expire it */
  curr += MAX_MEASUREMENT_AGE + 1;
  dirserv_expire_measured_bw_cache(curr);
  /* Check that the cache is empty */
  tt_int_op(dirserv_get_measured_bw_cache_size(),OP_EQ, 0);
  /* Check that we can't retrieve it */
  tt_assert(!dirserv_query_measured_bw_cache_kb(mbwl[0].node_id, NULL,NULL));
  /* Try caching a few things now */
  dirserv_cache_measured_bw(&(mbwl[0]), curr);
  tt_int_op(dirserv_get_measured_bw_cache_size(),OP_EQ, 1);
  curr += MAX_MEASUREMENT_AGE / 4;
  dirserv_cache_measured_bw(&(mbwl[1]), curr);
  tt_int_op(dirserv_get_measured_bw_cache_size(),OP_EQ, 2);
  curr += MAX_MEASUREMENT_AGE / 4;
  dirserv_cache_measured_bw(&(mbwl[2]), curr);
  tt_int_op(dirserv_get_measured_bw_cache_size(),OP_EQ, 3);
  curr += MAX_MEASUREMENT_AGE / 4 + 1;
  /* Do an expire that's too soon to get any of them */
  dirserv_expire_measured_bw_cache(curr);
  tt_int_op(dirserv_get_measured_bw_cache_size(),OP_EQ, 3);
  /* Push the oldest one off the cliff */
  curr += MAX_MEASUREMENT_AGE / 4;
  dirserv_expire_measured_bw_cache(curr);
  tt_int_op(dirserv_get_measured_bw_cache_size(),OP_EQ, 2);
  /* And another... */
  curr += MAX_MEASUREMENT_AGE / 4;
  dirserv_expire_measured_bw_cache(curr);
  tt_int_op(dirserv_get_measured_bw_cache_size(),OP_EQ, 1);
  /* This should empty it out again */
  curr += MAX_MEASUREMENT_AGE / 4;
  dirserv_expire_measured_bw_cache(curr);
  tt_int_op(dirserv_get_measured_bw_cache_size(),OP_EQ, 0);

 done:
  return;
}

static char *
my_dirvote_compute_params(smartlist_t *votes, int method,
                          int total_authorities)
{
  smartlist_t *s = dirvote_compute_params(votes, method, total_authorities);
  tor_assert(s);
  char *res = smartlist_join_strings(s, " ", 0, NULL);
  SMARTLIST_FOREACH(s, char *, cp, tor_free(cp));
  smartlist_free(s);
  return res;
}

#define dirvote_compute_params my_dirvote_compute_params

static void
test_dir_param_voting(void *arg)
{
  networkstatus_t vote1, vote2, vote3, vote4;
  smartlist_t *votes = smartlist_new();
  char *res = NULL;

  /* dirvote_compute_params only looks at the net_params field of the votes,
     so that's all we need to set.
   */
  (void)arg;
  memset(&vote1, 0, sizeof(vote1));
  memset(&vote2, 0, sizeof(vote2));
  memset(&vote3, 0, sizeof(vote3));
  memset(&vote4, 0, sizeof(vote4));
  vote1.net_params = smartlist_new();
  vote2.net_params = smartlist_new();
  vote3.net_params = smartlist_new();
  vote4.net_params = smartlist_new();
  smartlist_split_string(vote1.net_params,
                         "ab=90 abcd=20 cw=50 x-yz=-99", NULL, 0, 0);
  smartlist_split_string(vote2.net_params,
                         "ab=27 cw=5 x-yz=88", NULL, 0, 0);
  smartlist_split_string(vote3.net_params,
                         "abcd=20 c=60 cw=500 x-yz=-9 zzzzz=101", NULL, 0, 0);
  smartlist_split_string(vote4.net_params,
                         "ab=900 abcd=200 c=1 cw=51 x-yz=100", NULL, 0, 0);
  tt_int_op(100,OP_EQ, networkstatus_get_param(&vote4, "x-yz", 50, 0, 300));
  tt_int_op(222,OP_EQ, networkstatus_get_param(&vote4, "foobar", 222, 0, 300));
  tt_int_op(80,OP_EQ, networkstatus_get_param(&vote4, "ab", 12, 0, 80));
  tt_int_op(-8,OP_EQ, networkstatus_get_param(&vote4, "ab", -12, -100, -8));
  tt_int_op(0,OP_EQ, networkstatus_get_param(&vote4, "foobar", 0, -100, 8));

  tt_int_op(100,OP_EQ, networkstatus_get_overridable_param(
                                        &vote4, -1, "x-yz", 50, 0, 300));
  tt_int_op(30,OP_EQ, networkstatus_get_overridable_param(
                                        &vote4, 30, "x-yz", 50, 0, 300));
  tt_int_op(0,OP_EQ, networkstatus_get_overridable_param(
                                        &vote4, -101, "foobar", 0, -100, 8));
  tt_int_op(-99,OP_EQ, networkstatus_get_overridable_param(
                                        &vote4, -99, "foobar", 0, -100, 8));

  smartlist_add(votes, &vote1);

  /* Do the first tests without adding all the other votes, for
   * networks without many dirauths. */

  res = dirvote_compute_params(votes, 12, 2);
  tt_str_op(res,OP_EQ, "");
  tor_free(res);

  res = dirvote_compute_params(votes, 12, 1);
  tt_str_op(res,OP_EQ, "ab=90 abcd=20 cw=50 x-yz=-99");
  tor_free(res);

  smartlist_add(votes, &vote2);

  res = dirvote_compute_params(votes, 12, 2);
  tt_str_op(res,OP_EQ, "ab=27 cw=5 x-yz=-99");
  tor_free(res);

  res = dirvote_compute_params(votes, 12, 3);
  tt_str_op(res,OP_EQ, "ab=27 cw=5 x-yz=-99");
  tor_free(res);

  res = dirvote_compute_params(votes, 12, 6);
  tt_str_op(res,OP_EQ, "");
  tor_free(res);

  smartlist_add(votes, &vote3);

  res = dirvote_compute_params(votes, 12, 3);
  tt_str_op(res,OP_EQ, "ab=27 abcd=20 cw=50 x-yz=-9");
  tor_free(res);

  res = dirvote_compute_params(votes, 12, 5);
  tt_str_op(res,OP_EQ, "cw=50 x-yz=-9");
  tor_free(res);

  res = dirvote_compute_params(votes, 12, 9);
  tt_str_op(res,OP_EQ, "cw=50 x-yz=-9");
  tor_free(res);

  smartlist_add(votes, &vote4);

  res = dirvote_compute_params(votes, 12, 4);
  tt_str_op(res,OP_EQ, "ab=90 abcd=20 cw=50 x-yz=-9");
  tor_free(res);

  res = dirvote_compute_params(votes, 12, 5);
  tt_str_op(res,OP_EQ, "ab=90 abcd=20 cw=50 x-yz=-9");
  tor_free(res);

  /* Test that the special-cased "at least three dirauths voted for
   * this param" logic works as expected. */
  res = dirvote_compute_params(votes, 12, 6);
  tt_str_op(res,OP_EQ, "ab=90 abcd=20 cw=50 x-yz=-9");
  tor_free(res);

  res = dirvote_compute_params(votes, 12, 10);
  tt_str_op(res,OP_EQ, "ab=90 abcd=20 cw=50 x-yz=-9");
  tor_free(res);

 done:
  tor_free(res);
  SMARTLIST_FOREACH(vote1.net_params, char *, cp, tor_free(cp));
  SMARTLIST_FOREACH(vote2.net_params, char *, cp, tor_free(cp));
  SMARTLIST_FOREACH(vote3.net_params, char *, cp, tor_free(cp));
  SMARTLIST_FOREACH(vote4.net_params, char *, cp, tor_free(cp));
  smartlist_free(vote1.net_params);
  smartlist_free(vote2.net_params);
  smartlist_free(vote3.net_params);
  smartlist_free(vote4.net_params);
  smartlist_free(votes);

  return;
}

static void
test_dir_param_voting_lookup(void *arg)
{
  (void)arg;
  smartlist_t *lst = smartlist_new();

  smartlist_split_string(lst,
                         "moomin=9 moomin=10 moomintroll=5 fred "
                         "jack= electricity=sdk opa=6z abc=9 abcd=99",
                         NULL, 0, 0);

  tt_int_op(1000,
            OP_EQ, dirvote_get_intermediate_param_value(lst, "ab", 1000));
  tt_int_op(9, OP_EQ, dirvote_get_intermediate_param_value(lst, "abc", 1000));
  tt_int_op(99, OP_EQ,
            dirvote_get_intermediate_param_value(lst, "abcd", 1000));

  /* moomin appears twice. That's a bug. */
  tor_capture_bugs_(1);
  tt_int_op(-100, OP_EQ,
            dirvote_get_intermediate_param_value(lst, "moomin", -100));
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_EQ, 1);
  tt_str_op(smartlist_get(tor_get_captured_bug_log_(), 0), OP_EQ,
            "n_found == 0");
  tor_end_capture_bugs_();
  /* There is no 'fred=', so that is treated as not existing. */
  tt_int_op(-100, OP_EQ,
            dirvote_get_intermediate_param_value(lst, "fred", -100));
  /* jack is truncated */
  tor_capture_bugs_(1);
  tt_int_op(-100, OP_EQ,
            dirvote_get_intermediate_param_value(lst, "jack", -100));
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_EQ, 1);
  tt_str_op(smartlist_get(tor_get_captured_bug_log_(), 0), OP_EQ,
            "!(! ok)");
  tor_end_capture_bugs_();
  /* electricity and opa aren't integers. */
  tor_capture_bugs_(1);
  tt_int_op(-100, OP_EQ,
            dirvote_get_intermediate_param_value(lst, "electricity", -100));
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_EQ, 1);
  tt_str_op(smartlist_get(tor_get_captured_bug_log_(), 0), OP_EQ,
            "!(! ok)");
  tor_end_capture_bugs_();

  tor_capture_bugs_(1);
  tt_int_op(-100, OP_EQ,
            dirvote_get_intermediate_param_value(lst, "opa", -100));
  tt_int_op(smartlist_len(tor_get_captured_bug_log_()), OP_EQ, 1);
  tt_str_op(smartlist_get(tor_get_captured_bug_log_(), 0), OP_EQ,
            "!(! ok)");
  tor_end_capture_bugs_();

 done:
  SMARTLIST_FOREACH(lst, char *, cp, tor_free(cp));
  smartlist_free(lst);
  tor_end_capture_bugs_();
}

#undef dirvote_compute_params

/** Helper: Test that two networkstatus_voter_info_t do in fact represent the
 * same voting authority, and that they do in fact have all the same
 * information. */
static void
test_same_voter(networkstatus_voter_info_t *v1,
                networkstatus_voter_info_t *v2)
{
  tt_str_op(v1->nickname,OP_EQ, v2->nickname);
  tt_mem_op(v1->identity_digest,OP_EQ, v2->identity_digest, DIGEST_LEN);
  tt_str_op(v1->address,OP_EQ, v2->address);
  tt_int_op(v1->addr,OP_EQ, v2->addr);
  tt_int_op(v1->dir_port,OP_EQ, v2->dir_port);
  tt_int_op(v1->or_port,OP_EQ, v2->or_port);
  tt_str_op(v1->contact,OP_EQ, v2->contact);
  tt_mem_op(v1->vote_digest,OP_EQ, v2->vote_digest, DIGEST_LEN);
 done:
  ;
}

/** Helper: get a detached signatures document for one or two
 * consensuses. */
static char *
get_detached_sigs(networkstatus_t *ns, networkstatus_t *ns2)
{
  char *r;
  smartlist_t *sl;
  tor_assert(ns && ns->flavor == FLAV_NS);
  sl = smartlist_new();
  smartlist_add(sl,ns);
  if (ns2)
    smartlist_add(sl,ns2);
  r = networkstatus_get_detached_signatures(sl);
  smartlist_free(sl);
  return r;
}

/** Apply tweaks to the vote list for each voter */
static int
vote_tweaks_for_v3ns(networkstatus_t *v, int voter, time_t now)
{
  vote_routerstatus_t *vrs;
  const char *msg = NULL;

  tt_assert(v);
  (void)now;

  if (voter == 1) {
    measured_bw_line_t mbw;
    memset(mbw.node_id, 33, sizeof(mbw.node_id));
    mbw.bw_kb = 1024;
    tt_int_op(measured_bw_line_apply(&mbw, v->routerstatus_list), OP_EQ, 1);
  } else if (voter == 2 || voter == 3) {
    /* Monkey around with the list a bit */
    vrs = smartlist_get(v->routerstatus_list, 2);
    smartlist_del_keeporder(v->routerstatus_list, 2);
    vote_routerstatus_free(vrs);
    vrs = smartlist_get(v->routerstatus_list, 0);
    vrs->status.is_fast = 1;

    if (voter == 3) {
      vrs = smartlist_get(v->routerstatus_list, 0);
      smartlist_del_keeporder(v->routerstatus_list, 0);
      vote_routerstatus_free(vrs);
      vrs = smartlist_get(v->routerstatus_list, 0);
      memset(vrs->status.descriptor_digest, (int)'Z', DIGEST_LEN);
      tt_assert(router_add_to_routerlist(
                  dir_common_generate_ri_from_rs(vrs), &msg,0,0) >= 0);
    }
  }

 done:
  return 0;
}

/**
 * Test a parsed vote_routerstatus_t for v3_networkstatus test
 */
static void
test_vrs_for_v3ns(vote_routerstatus_t *vrs, int voter, time_t now)
{
  routerstatus_t *rs;
  tor_addr_t addr_ipv6;

  tt_assert(vrs);
  rs = &(vrs->status);
  tt_assert(rs);

  /* Split out by digests to test */
  if (tor_memeq(rs->identity_digest,
                "\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3"
                "\x3\x3\x3\x3",
                DIGEST_LEN) &&
                (voter == 1)) {
    /* Check the first routerstatus. */
    tt_str_op(vrs->version,OP_EQ, "0.1.2.14");
    tt_int_op(rs->published_on,OP_EQ, now-1500);
    tt_str_op(rs->nickname,OP_EQ, "router2");
    tt_mem_op(rs->identity_digest,OP_EQ,
               "\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3"
               "\x3\x3\x3\x3",
               DIGEST_LEN);
    tt_mem_op(rs->descriptor_digest,OP_EQ, "NNNNNNNNNNNNNNNNNNNN", DIGEST_LEN);
    tt_int_op(rs->addr,OP_EQ, 0x99008801);
    tt_int_op(rs->or_port,OP_EQ, 443);
    tt_int_op(rs->dir_port,OP_EQ, 8000);
    /* no flags except "running" (16) and "v2dir" (64) and "valid" (128) */
    tt_u64_op(vrs->flags, OP_EQ, UINT64_C(0xd0));
  } else if (tor_memeq(rs->identity_digest,
                       "\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5"
                       "\x5\x5\x5\x5",
                       DIGEST_LEN) &&
                       (voter == 1 || voter == 2)) {
    tt_mem_op(rs->identity_digest,OP_EQ,
               "\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5"
               "\x5\x5\x5\x5",
               DIGEST_LEN);

    if (voter == 1) {
      /* Check the second routerstatus. */
      tt_str_op(vrs->version,OP_EQ, "0.2.0.5");
      tt_int_op(rs->published_on,OP_EQ, now-1000);
      tt_str_op(rs->nickname,OP_EQ, "router1");
    }
    tt_mem_op(rs->descriptor_digest,OP_EQ, "MMMMMMMMMMMMMMMMMMMM", DIGEST_LEN);
    tt_int_op(rs->addr,OP_EQ, 0x99009901);
    tt_int_op(rs->or_port,OP_EQ, 443);
    tt_int_op(rs->dir_port,OP_EQ, 0);
    tor_addr_parse(&addr_ipv6, "[1:2:3::4]");
    tt_assert(tor_addr_eq(&rs->ipv6_addr, &addr_ipv6));
    tt_int_op(rs->ipv6_orport,OP_EQ, 4711);
    if (voter == 1) {
      /* all except "authority" (1) */
      tt_u64_op(vrs->flags, OP_EQ, UINT64_C(254));
    } else {
      /* 1023 - authority(1) - madeofcheese(16) - madeoftin(32) */
      tt_u64_op(vrs->flags, OP_EQ, UINT64_C(974));
    }
  } else if (tor_memeq(rs->identity_digest,
                       "\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33"
                       "\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33",
                       DIGEST_LEN) &&
                       (voter == 1 || voter == 2)) {
    /* Check the measured bandwidth bits */
    tt_assert(vrs->has_measured_bw &&
                vrs->measured_bw_kb == 1024);
  } else {
    /*
     * Didn't expect this, but the old unit test only checked some of them,
     * so don't assert.
     */
    /* tt_assert(0); */
  }

 done:
  return;
}

/**
 * Test a consensus for v3_networkstatus_test
 */
static void
test_consensus_for_v3ns(networkstatus_t *con, time_t now)
{
  (void)now;

  tt_assert(con);
  tt_ptr_op(con->cert, OP_EQ, NULL);
  tt_int_op(2,OP_EQ, smartlist_len(con->routerstatus_list));
  /* There should be two listed routers: one with identity 3, one with
   * identity 5. */

 done:
  return;
}

/**
 * Test a router list entry for v3_networkstatus test
 */
static void
test_routerstatus_for_v3ns(routerstatus_t *rs, time_t now)
{
  tor_addr_t addr_ipv6;

  tt_assert(rs);

  /* There should be two listed routers: one with identity 3, one with
   * identity 5. */
  /* This one showed up in 2 digests. */
  if (tor_memeq(rs->identity_digest,
                "\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3"
                "\x3\x3",
                DIGEST_LEN)) {
    tt_mem_op(rs->identity_digest,OP_EQ,
               "\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3",
               DIGEST_LEN);
    tt_mem_op(rs->descriptor_digest,OP_EQ, "NNNNNNNNNNNNNNNNNNNN", DIGEST_LEN);
    tt_assert(!rs->is_authority);
    tt_assert(!rs->is_exit);
    tt_assert(!rs->is_fast);
    tt_assert(!rs->is_possible_guard);
    tt_assert(!rs->is_stable);
    /* (If it wasn't running it wouldn't be here) */
    tt_assert(rs->is_flagged_running);
    tt_assert(rs->is_valid);
    tt_assert(!rs->is_named);
    tt_assert(rs->is_v2_dir);
    /* XXXX check version */
  } else if (tor_memeq(rs->identity_digest,
                       "\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5"
                       "\x5\x5\x5\x5",
                       DIGEST_LEN)) {
    /* This one showed up in 3 digests. Twice with ID 'M', once with 'Z'.  */
    tt_mem_op(rs->identity_digest,OP_EQ,
               "\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5",
               DIGEST_LEN);
    tt_str_op(rs->nickname,OP_EQ, "router1");
    tt_mem_op(rs->descriptor_digest,OP_EQ, "MMMMMMMMMMMMMMMMMMMM", DIGEST_LEN);
    tt_int_op(rs->published_on,OP_EQ, now-1000);
    tt_int_op(rs->addr,OP_EQ, 0x99009901);
    tt_int_op(rs->or_port,OP_EQ, 443);
    tt_int_op(rs->dir_port,OP_EQ, 0);
    tor_addr_parse(&addr_ipv6, "[1:2:3::4]");
    tt_assert(tor_addr_eq(&rs->ipv6_addr, &addr_ipv6));
    tt_int_op(rs->ipv6_orport,OP_EQ, 4711);
    tt_assert(!rs->is_authority);
    tt_assert(rs->is_exit);
    tt_assert(rs->is_fast);
    tt_assert(rs->is_possible_guard);
    tt_assert(rs->is_stable);
    tt_assert(rs->is_flagged_running);
    tt_assert(rs->is_valid);
    tt_assert(rs->is_v2_dir);
    tt_assert(!rs->is_named);
    /* XXXX check version */
  } else {
    /* Weren't expecting this... */
    tt_abort();
  }

 done:
  return;
}

static void
test_dir_networkstatus_compute_bw_weights_v10(void *arg)
{
  (void) arg;
  smartlist_t *chunks = smartlist_new();
  int64_t G, M, E, D, T, weight_scale;
  int ret;
  weight_scale = 10000;

  /* no case. one or more of the values is 0 */
  G = M = E = D = 0;
  T = G + M + E + D;
  ret = networkstatus_compute_bw_weights_v10(chunks, G, M, E, D, T,
                                             weight_scale);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(smartlist_len(chunks), OP_EQ, 0);

  /* case 1 */
  /* XXX dir-spec not followed? See #20272. If it isn't closed, then this is
   * testing current behavior, not spec. */
  G = E = 10;
  M = D = 1;
  T = G + M + E + D;
  ret = networkstatus_compute_bw_weights_v10(chunks, G, M, E, D, T,
                                             weight_scale);
  tt_int_op(ret, OP_EQ, 1);
  tt_int_op(smartlist_len(chunks), OP_EQ, 1);
  tt_str_op(smartlist_get(chunks, 0), OP_EQ, "bandwidth-weights Wbd=3333 "
    "Wbe=3000 Wbg=3000 Wbm=10000 Wdb=10000 Web=10000 Wed=3333 Wee=7000 "
    "Weg=3333 Wem=7000 Wgb=10000 Wgd=3333 Wgg=7000 Wgm=7000 Wmb=10000 "
    "Wmd=3333 Wme=3000 Wmg=3000 Wmm=10000\n");
  SMARTLIST_FOREACH(chunks, char *, cp, tor_free(cp));
  smartlist_clear(chunks);

  /* case 2a E scarce */
  M = 100;
  G = 20;
  E = D = 5;
  T = G + M + E + D;
  ret = networkstatus_compute_bw_weights_v10(chunks, G, M, E, D, T,
                                             weight_scale);
  tt_int_op(ret, OP_EQ, 1);
  tt_str_op(smartlist_get(chunks, 0), OP_EQ, "bandwidth-weights Wbd=0 Wbe=0 "
    "Wbg=0 Wbm=10000 Wdb=10000 Web=10000 Wed=10000 Wee=10000 Weg=10000 "
    "Wem=10000 Wgb=10000 Wgd=0 Wgg=10000 Wgm=10000 Wmb=10000 Wmd=0 Wme=0 "
    "Wmg=0 Wmm=10000\n");
  SMARTLIST_FOREACH(chunks, char *, cp, tor_free(cp));
  smartlist_clear(chunks);

  /* case 2a G scarce */
  M = 100;
  E = 20;
  G = D = 5;
  T = G + M + E + D;
  ret = networkstatus_compute_bw_weights_v10(chunks, G, M, E, D, T,
                                             weight_scale);
  tt_int_op(ret, OP_EQ, 1);
  tt_str_op(smartlist_get(chunks, 0), OP_EQ, "bandwidth-weights Wbd=0 Wbe=0 "
    "Wbg=0 Wbm=10000 Wdb=10000 Web=10000 Wed=0 Wee=10000 Weg=0 Wem=10000 "
    "Wgb=10000 Wgd=10000 Wgg=10000 Wgm=10000 Wmb=10000 Wmd=0 Wme=0 Wmg=0 "
    "Wmm=10000\n");
  SMARTLIST_FOREACH(chunks, char *, cp, tor_free(cp));
  smartlist_clear(chunks);

  /* case 2b1 (Wgg=1, Wmd=Wgd) */
  M = 10;
  E = 30;
  G = 10;
  D = 100;
  T = G + M + E + D;
  ret = networkstatus_compute_bw_weights_v10(chunks, G, M, E, D, T,
                                             weight_scale);
  tt_int_op(ret, OP_EQ, 1);
  tt_str_op(smartlist_get(chunks, 0), OP_EQ, "bandwidth-weights Wbd=4000 "
    "Wbe=0 Wbg=0 Wbm=10000 Wdb=10000 Web=10000 Wed=2000 Wee=10000 Weg=2000 "
    "Wem=10000 Wgb=10000 Wgd=4000 Wgg=10000 Wgm=10000 Wmb=10000 Wmd=4000 "
    "Wme=0 Wmg=0 Wmm=10000\n");
  SMARTLIST_FOREACH(chunks, char *, cp, tor_free(cp));
  smartlist_clear(chunks);

  /* case 2b2 */
  M = 60;
  E = 30;
  G = 10;
  D = 100;
  T = G + M + E + D;
  ret = networkstatus_compute_bw_weights_v10(chunks, G, M, E, D, T,
                                             weight_scale);
  tt_int_op(ret, OP_EQ, 1);
  tt_str_op(smartlist_get(chunks, 0), OP_EQ, "bandwidth-weights Wbd=666 Wbe=0 "
    "Wbg=0 Wbm=10000 Wdb=10000 Web=10000 Wed=3666 Wee=10000 Weg=3666 "
    "Wem=10000 Wgb=10000 Wgd=5668 Wgg=10000 Wgm=10000 Wmb=10000 Wmd=666 "
    "Wme=0 Wmg=0 Wmm=10000\n");
  SMARTLIST_FOREACH(chunks, char *, cp, tor_free(cp));
  smartlist_clear(chunks);

  /* case 2b3 */
  /* XXX I can't get a combination of values that hits this case without error,
   * so this just tests that it fails. See #20285. Also see #20284 as 2b3 does
   * not follow dir-spec. */
  /* (E < T/3 && G < T/3) && (E+D>=G || G+D>=E) && (M > T/3) */
  M = 80;
  E = 30;
  G = 30;
  D = 30;
  T = G + M + E + D;
  ret = networkstatus_compute_bw_weights_v10(chunks, G, M, E, D, T,
                                             weight_scale);
  tt_int_op(ret, OP_EQ, 0);
  SMARTLIST_FOREACH(chunks, char *, cp, tor_free(cp));
  smartlist_clear(chunks);

  /* case 3a G scarce */
  M = 10;
  E = 30;
  G = 10;
  D = 5;
  T = G + M + E + D;
  ret = networkstatus_compute_bw_weights_v10(chunks, G, M, E, D, T,
                                             weight_scale);
  tt_int_op(ret, OP_EQ, 1);
  tt_str_op(smartlist_get(chunks, 0), OP_EQ, "bandwidth-weights Wbd=0 "
    "Wbe=3333 Wbg=0 Wbm=10000 Wdb=10000 Web=10000 Wed=0 Wee=6667 Weg=0 "
    "Wem=6667 Wgb=10000 Wgd=10000 Wgg=10000 Wgm=10000 Wmb=10000 Wmd=0 "
    "Wme=3333 Wmg=0 Wmm=10000\n");
  SMARTLIST_FOREACH(chunks, char *, cp, tor_free(cp));
  smartlist_clear(chunks);

  /* case 3a E scarce */
  M = 10;
  E = 10;
  G = 30;
  D = 5;
  T = G + M + E + D;
  ret = networkstatus_compute_bw_weights_v10(chunks, G, M, E, D, T,
                                             weight_scale);
  tt_int_op(ret, OP_EQ, 1);
  tt_str_op(smartlist_get(chunks, 0), OP_EQ, "bandwidth-weights Wbd=0 Wbe=0 "
    "Wbg=3333 Wbm=10000 Wdb=10000 Web=10000 Wed=10000 Wee=10000 Weg=10000 "
    "Wem=10000 Wgb=10000 Wgd=0 Wgg=6667 Wgm=6667 Wmb=10000 Wmd=0 Wme=0 "
    "Wmg=3333 Wmm=10000\n");
  SMARTLIST_FOREACH(chunks, char *, cp, tor_free(cp));
  smartlist_clear(chunks);

  /* case 3bg */
  M = 10;
  E = 30;
  G = 10;
  D = 10;
  T = G + M + E + D;
  ret = networkstatus_compute_bw_weights_v10(chunks, G, M, E, D, T,
                                             weight_scale);
  tt_int_op(ret, OP_EQ, 1);
  tt_str_op(smartlist_get(chunks, 0), OP_EQ, "bandwidth-weights Wbd=0 "
    "Wbe=3334 Wbg=0 Wbm=10000 Wdb=10000 Web=10000 Wed=0 Wee=6666 Weg=0 "
    "Wem=6666 Wgb=10000 Wgd=10000 Wgg=10000 Wgm=10000 Wmb=10000 Wmd=0 "
    "Wme=3334 Wmg=0 Wmm=10000\n");
  SMARTLIST_FOREACH(chunks, char *, cp, tor_free(cp));
  smartlist_clear(chunks);

  /* case 3be */
  M = 10;
  E = 10;
  G = 30;
  D = 10;
  T = G + M + E + D;
  ret = networkstatus_compute_bw_weights_v10(chunks, G, M, E, D, T,
                                             weight_scale);
  tt_int_op(ret, OP_EQ, 1);
  tt_str_op(smartlist_get(chunks, 0), OP_EQ, "bandwidth-weights Wbd=0 Wbe=0 "
    "Wbg=3334 Wbm=10000 Wdb=10000 Web=10000 Wed=10000 Wee=10000 Weg=10000 "
    "Wem=10000 Wgb=10000 Wgd=0 Wgg=6666 Wgm=6666 Wmb=10000 Wmd=0 Wme=0 "
    "Wmg=3334 Wmm=10000\n");
  SMARTLIST_FOREACH(chunks, char *, cp, tor_free(cp));
  smartlist_clear(chunks);

  /* case from 21 Jul 2013 (3be) */
  G = 5483409;
  M = 1455379;
  E = 980834;
  D = 3385803;
  T = 11305425;
  tt_i64_op(G+M+E+D, OP_EQ, T);
  ret = networkstatus_compute_bw_weights_v10(chunks, G, M, E, D, T,
                                             weight_scale);
  tt_assert(ret);
  tt_str_op(smartlist_get(chunks, 0), OP_EQ, "bandwidth-weights Wbd=883 Wbe=0 "
    "Wbg=3673 Wbm=10000 Wdb=10000 Web=10000 Wed=8233 Wee=10000 Weg=8233 "
    "Wem=10000 Wgb=10000 Wgd=883 Wgg=6327 Wgm=6327 Wmb=10000 Wmd=883 Wme=0 "
    "Wmg=3673 Wmm=10000\n");
  SMARTLIST_FOREACH(chunks, char *, cp, tor_free(cp));
  smartlist_clear(chunks);

  /* case from 04 Oct 2016 (3a E scarce) */
  G=29322240;
  M=4721546;
  E=1522058;
  D=9273571;
  T=44839415;
  tt_i64_op(G+M+E+D, OP_EQ, T);
  ret = networkstatus_compute_bw_weights_v10(chunks, G, M, E, D, T,
                                             weight_scale);
  tt_assert(ret);
  tt_str_op(smartlist_get(chunks, 0), OP_EQ, "bandwidth-weights Wbd=0 Wbe=0 "
    "Wbg=4194 Wbm=10000 Wdb=10000 Web=10000 Wed=10000 Wee=10000 Weg=10000 "
    "Wem=10000 Wgb=10000 Wgd=0 Wgg=5806 Wgm=5806 Wmb=10000 Wmd=0 Wme=0 "
    "Wmg=4194 Wmm=10000\n");
  SMARTLIST_FOREACH(chunks, char *, cp, tor_free(cp));
  smartlist_clear(chunks);

  /* case from 04 Sep 2013 (2b1) */
  G=3091352;
  M=1838837;
  E=2109300;
  D=2469369;
  T=9508858;
  tt_i64_op(G+M+E+D, OP_EQ, T);
  ret = networkstatus_compute_bw_weights_v10(chunks, G, M, E, D, T,
                                             weight_scale);
  tt_assert(ret);
  tt_str_op(smartlist_get(chunks, 0), OP_EQ, "bandwidth-weights Wbd=317 "
    "Wbe=5938 Wbg=0 Wbm=10000 Wdb=10000 Web=10000 Wed=9366 Wee=4061 "
    "Weg=9366 Wem=4061 Wgb=10000 Wgd=317 Wgg=10000 Wgm=10000 Wmb=10000 "
    "Wmd=317 Wme=5938 Wmg=0 Wmm=10000\n");
  SMARTLIST_FOREACH(chunks, char *, cp, tor_free(cp));
  smartlist_clear(chunks);

  /* explicitly test initializing weights to 1*/
  G=1;
  M=1;
  E=1;
  D=1;
  T=4;
  tt_i64_op(G+M+E+D, OP_EQ, T);
  ret = networkstatus_compute_bw_weights_v10(chunks, G, M, E, D, T,
                                             weight_scale);
  tt_str_op(smartlist_get(chunks, 0), OP_EQ, "bandwidth-weights Wbd=3333 "
    "Wbe=0 Wbg=0 Wbm=10000 Wdb=10000 Web=10000 Wed=3333 Wee=10000 Weg=3333 "
    "Wem=10000 Wgb=10000 Wgd=3333 Wgg=10000 Wgm=10000 Wmb=10000 Wmd=3333 "
    "Wme=0 Wmg=0 Wmm=10000\n");
  tt_assert(ret);

 done:
  SMARTLIST_FOREACH(chunks, char *, cp, tor_free(cp));
  smartlist_free(chunks);
}

static authority_cert_t *mock_cert;

static authority_cert_t *
get_my_v3_authority_cert_m(void)
{
  tor_assert(mock_cert);
  return mock_cert;
}

/** Run a unit tests for generating and parsing networkstatuses, with
 * the supply test fns. */
static void
test_a_networkstatus(
    vote_routerstatus_t * (*vrs_gen)(int idx, time_t now),
    int (*vote_tweaks)(networkstatus_t *v, int voter, time_t now),
    void (*vrs_test)(vote_routerstatus_t *vrs, int voter, time_t now),
    void (*consensus_test)(networkstatus_t *con, time_t now),
    void (*rs_test)(routerstatus_t *rs, time_t now))
{
  authority_cert_t *cert1=NULL, *cert2=NULL, *cert3=NULL;
  crypto_pk_t *sign_skey_1=NULL, *sign_skey_2=NULL, *sign_skey_3=NULL;
  crypto_pk_t *sign_skey_leg1=NULL;
  /*
   * Sum the non-zero returns from vote_tweaks() we've seen; if vote_tweaks()
   * returns non-zero, it changed net_params and we should skip the tests for
   * that later as they will fail.
   */
  int params_tweaked = 0;

  time_t now = time(NULL);
  networkstatus_voter_info_t *voter;
  document_signature_t *sig;
  networkstatus_t *vote=NULL, *v1=NULL, *v2=NULL, *v3=NULL, *con=NULL,
    *con_md=NULL;
  vote_routerstatus_t *vrs;
  routerstatus_t *rs;
  int idx, n_rs, n_vrs;
  char *consensus_text=NULL, *cp=NULL;
  smartlist_t *votes = smartlist_new();

  /* For generating the two other consensuses. */
  char *detached_text1=NULL, *detached_text2=NULL;
  char *consensus_text2=NULL, *consensus_text3=NULL;
  char *consensus_text_md2=NULL, *consensus_text_md3=NULL;
  char *consensus_text_md=NULL;
  networkstatus_t *con2=NULL, *con_md2=NULL, *con3=NULL, *con_md3=NULL;
  ns_detached_signatures_t *dsig1=NULL, *dsig2=NULL;

  tt_assert(vrs_gen);
  tt_assert(rs_test);
  tt_assert(vrs_test);

  MOCK(get_my_v3_authority_cert, get_my_v3_authority_cert_m);

  /* Parse certificates and keys. */
  cert1 = mock_cert = authority_cert_parse_from_string(AUTHORITY_CERT_1,
                                                 strlen(AUTHORITY_CERT_1),
                                                 NULL);
  tt_assert(cert1);
  cert2 = authority_cert_parse_from_string(AUTHORITY_CERT_2,
                                           strlen(AUTHORITY_CERT_2),
                                           NULL);
  tt_assert(cert2);
  cert3 = authority_cert_parse_from_string(AUTHORITY_CERT_3,
                                           strlen(AUTHORITY_CERT_3),
                                           NULL);
  tt_assert(cert3);
  sign_skey_1 = crypto_pk_new();
  sign_skey_2 = crypto_pk_new();
  sign_skey_3 = crypto_pk_new();
  sign_skey_leg1 = pk_generate(4);
  voting_schedule_recalculate_timing(get_options(), now);
  sr_state_init(0, 0);

  tt_assert(!crypto_pk_read_private_key_from_string(sign_skey_1,
                                                   AUTHORITY_SIGNKEY_1, -1));
  tt_assert(!crypto_pk_read_private_key_from_string(sign_skey_2,
                                                   AUTHORITY_SIGNKEY_2, -1));
  tt_assert(!crypto_pk_read_private_key_from_string(sign_skey_3,
                                                   AUTHORITY_SIGNKEY_3, -1));

  tt_assert(!crypto_pk_cmp_keys(sign_skey_1, cert1->signing_key));
  tt_assert(!crypto_pk_cmp_keys(sign_skey_2, cert2->signing_key));

  tt_assert(!dir_common_construct_vote_1(&vote, cert1, sign_skey_1, vrs_gen,
                                         &v1, &n_vrs, now, 1));
  tt_assert(v1);

  /* Make sure the parsed thing was right. */
  tt_int_op(v1->type,OP_EQ, NS_TYPE_VOTE);
  tt_int_op(v1->published,OP_EQ, vote->published);
  tt_int_op(v1->valid_after,OP_EQ, vote->valid_after);
  tt_int_op(v1->fresh_until,OP_EQ, vote->fresh_until);
  tt_int_op(v1->valid_until,OP_EQ, vote->valid_until);
  tt_int_op(v1->vote_seconds,OP_EQ, vote->vote_seconds);
  tt_int_op(v1->dist_seconds,OP_EQ, vote->dist_seconds);
  tt_str_op(v1->client_versions,OP_EQ, vote->client_versions);
  tt_str_op(v1->server_versions,OP_EQ, vote->server_versions);
  tt_assert(v1->voters && smartlist_len(v1->voters));
  voter = smartlist_get(v1->voters, 0);
  tt_str_op(voter->nickname,OP_EQ, "Voter1");
  tt_str_op(voter->address,OP_EQ, "1.2.3.4");
  tt_int_op(voter->addr,OP_EQ, 0x01020304);
  tt_int_op(voter->dir_port,OP_EQ, 80);
  tt_int_op(voter->or_port,OP_EQ, 9000);
  tt_str_op(voter->contact,OP_EQ, "voter@example.com");
  tt_assert(v1->cert);
  tt_assert(!crypto_pk_cmp_keys(sign_skey_1, v1->cert->signing_key));
  cp = smartlist_join_strings(v1->known_flags, ":", 0, NULL);
  tt_str_op(cp,OP_EQ, "Authority:Exit:Fast:Guard:Running:Stable:V2Dir:Valid");
  tor_free(cp);
  tt_int_op(smartlist_len(v1->routerstatus_list),OP_EQ, n_vrs);
  networkstatus_vote_free(vote);
  vote = NULL;

  if (vote_tweaks) params_tweaked += vote_tweaks(v1, 1, now);

  /* Check the routerstatuses. */
  for (idx = 0; idx < n_vrs; ++idx) {
    vrs = smartlist_get(v1->routerstatus_list, idx);
    tt_assert(vrs);
    vrs_test(vrs, 1, now);
  }

  /* Generate second vote. It disagrees on some of the times,
   * and doesn't list versions, and knows some crazy flags.
   * Generate and parse v2. */
  tt_assert(!dir_common_construct_vote_2(&vote, cert2, sign_skey_2, vrs_gen,
                                         &v2, &n_vrs, now, 1));
  tt_assert(v2);

  if (vote_tweaks) params_tweaked += vote_tweaks(v2, 2, now);

  /* Check that flags come out right.*/
  cp = smartlist_join_strings(v2->known_flags, ":", 0, NULL);
  tt_str_op(cp,OP_EQ, "Authority:Exit:Fast:Guard:MadeOfCheese:MadeOfTin:"
             "Running:Stable:V2Dir:Valid");
  tor_free(cp);

  /* Check the routerstatuses. */
  n_vrs = smartlist_len(v2->routerstatus_list);
  for (idx = 0; idx < n_vrs; ++idx) {
    vrs = smartlist_get(v2->routerstatus_list, idx);
    tt_assert(vrs);
    vrs_test(vrs, 2, now);
  }
  networkstatus_vote_free(vote);
  vote = NULL;

  /* Generate the third vote with a legacy id. */
  tt_assert(!dir_common_construct_vote_3(&vote, cert3, sign_skey_3, vrs_gen,
                                         &v3, &n_vrs, now, 1));
  tt_assert(v3);

  if (vote_tweaks) params_tweaked += vote_tweaks(v3, 3, now);

  /* Compute a consensus as voter 3. */
  smartlist_add(votes, v3);
  smartlist_add(votes, v1);
  smartlist_add(votes, v2);
  consensus_text = networkstatus_compute_consensus(votes, 3,
                                                   cert3->identity_key,
                                                   sign_skey_3,
                                                   "AAAAAAAAAAAAAAAAAAAA",
                                                   sign_skey_leg1,
                                                   FLAV_NS);
  tt_assert(consensus_text);
  con = networkstatus_parse_vote_from_string_(consensus_text, NULL,
                                             NS_TYPE_CONSENSUS);
  tt_assert(con);
  //log_notice(LD_GENERAL, "<<%s>>\n<<%s>>\n<<%s>>\n",
  //           v1_text, v2_text, v3_text);
  consensus_text_md = networkstatus_compute_consensus(votes, 3,
                                                   cert3->identity_key,
                                                   sign_skey_3,
                                                   "AAAAAAAAAAAAAAAAAAAA",
                                                   sign_skey_leg1,
                                                   FLAV_MICRODESC);
  tt_assert(consensus_text_md);
  con_md = networkstatus_parse_vote_from_string_(consensus_text_md, NULL,
                                                NS_TYPE_CONSENSUS);
  tt_assert(con_md);
  tt_int_op(con_md->flavor,OP_EQ, FLAV_MICRODESC);

  /* Check consensus contents. */
  tt_assert(con->type == NS_TYPE_CONSENSUS);
  tt_int_op(con->published,OP_EQ, 0); /* this field only appears in votes. */
  tt_int_op(con->valid_after,OP_EQ, now+1000);
  tt_int_op(con->fresh_until,OP_EQ, now+2003); /* median */
  tt_int_op(con->valid_until,OP_EQ, now+3000);
  tt_int_op(con->vote_seconds,OP_EQ, 100);
  tt_int_op(con->dist_seconds,OP_EQ, 250); /* median */
  tt_str_op(con->client_versions,OP_EQ, "0.1.2.14");
  tt_str_op(con->server_versions,OP_EQ, "0.1.2.15,0.1.2.16");
  cp = smartlist_join_strings(v2->known_flags, ":", 0, NULL);
  tt_str_op(cp,OP_EQ, "Authority:Exit:Fast:Guard:MadeOfCheese:MadeOfTin:"
             "Running:Stable:V2Dir:Valid");
  tor_free(cp);
  if (!params_tweaked) {
    /* Skip this one if vote_tweaks() messed with the param lists */
    cp = smartlist_join_strings(con->net_params, ":", 0, NULL);
    tt_str_op(cp,OP_EQ, "circuitwindow=80:foo=660");
    tor_free(cp);
  }

  tt_int_op(4,OP_EQ, smartlist_len(con->voters)); /*3 voters, 1 legacy key.*/
  /* The voter id digests should be in this order. */
  tt_assert(fast_memcmp(cert2->cache_info.identity_digest,
                     cert1->cache_info.identity_digest,DIGEST_LEN)<0);
  tt_assert(fast_memcmp(cert1->cache_info.identity_digest,
                     cert3->cache_info.identity_digest,DIGEST_LEN)<0);
  test_same_voter(smartlist_get(con->voters, 1),
                  smartlist_get(v2->voters, 0));
  test_same_voter(smartlist_get(con->voters, 2),
                  smartlist_get(v1->voters, 0));
  test_same_voter(smartlist_get(con->voters, 3),
                  smartlist_get(v3->voters, 0));

  consensus_test(con, now);

  /* Check the routerstatuses. */
  n_rs = smartlist_len(con->routerstatus_list);
  tt_assert(n_rs);
  for (idx = 0; idx < n_rs; ++idx) {
    rs = smartlist_get(con->routerstatus_list, idx);
    tt_assert(rs);
    rs_test(rs, now);
  }

  n_rs = smartlist_len(con_md->routerstatus_list);
  tt_assert(n_rs);
  for (idx = 0; idx < n_rs; ++idx) {
    rs = smartlist_get(con_md->routerstatus_list, idx);
    tt_assert(rs);
  }

  /* Check signatures.  the first voter is a pseudo-entry with a legacy key.
   * The second one hasn't signed.  The fourth one has signed: validate it. */
  voter = smartlist_get(con->voters, 1);
  tt_int_op(smartlist_len(voter->sigs),OP_EQ, 0);

  voter = smartlist_get(con->voters, 3);
  tt_int_op(smartlist_len(voter->sigs),OP_EQ, 1);
  sig = smartlist_get(voter->sigs, 0);
  tt_assert(sig->signature);
  tt_assert(!sig->good_signature);
  tt_assert(!sig->bad_signature);

  tt_assert(!networkstatus_check_document_signature(con, sig, cert3));
  tt_assert(sig->signature);
  tt_assert(sig->good_signature);
  tt_assert(!sig->bad_signature);

  {
    const char *msg=NULL;
    /* Compute the other two signed consensuses. */
    smartlist_shuffle(votes);
    consensus_text2 = networkstatus_compute_consensus(votes, 3,
                                                      cert2->identity_key,
                                                      sign_skey_2, NULL,NULL,
                                                      FLAV_NS);
    consensus_text_md2 = networkstatus_compute_consensus(votes, 3,
                                                      cert2->identity_key,
                                                      sign_skey_2, NULL,NULL,
                                                      FLAV_MICRODESC);
    smartlist_shuffle(votes);
    consensus_text3 = networkstatus_compute_consensus(votes, 3,
                                                      cert1->identity_key,
                                                      sign_skey_1, NULL,NULL,
                                                      FLAV_NS);
    consensus_text_md3 = networkstatus_compute_consensus(votes, 3,
                                                      cert1->identity_key,
                                                      sign_skey_1, NULL,NULL,
                                                      FLAV_MICRODESC);
    tt_assert(consensus_text2);
    tt_assert(consensus_text3);
    tt_assert(consensus_text_md2);
    tt_assert(consensus_text_md3);
    con2 = networkstatus_parse_vote_from_string_(consensus_text2, NULL,
                                                NS_TYPE_CONSENSUS);
    con3 = networkstatus_parse_vote_from_string_(consensus_text3, NULL,
                                                NS_TYPE_CONSENSUS);
    con_md2 = networkstatus_parse_vote_from_string_(consensus_text_md2, NULL,
                                                NS_TYPE_CONSENSUS);
    con_md3 = networkstatus_parse_vote_from_string_(consensus_text_md3, NULL,
                                                NS_TYPE_CONSENSUS);
    tt_assert(con2);
    tt_assert(con3);
    tt_assert(con_md2);
    tt_assert(con_md3);

    /* All three should have the same digest. */
    tt_mem_op(&con->digests,OP_EQ, &con2->digests, sizeof(common_digests_t));
    tt_mem_op(&con->digests,OP_EQ, &con3->digests, sizeof(common_digests_t));

    tt_mem_op(&con_md->digests,OP_EQ, &con_md2->digests,
              sizeof(common_digests_t));
    tt_mem_op(&con_md->digests,OP_EQ, &con_md3->digests,
              sizeof(common_digests_t));

    /* Extract a detached signature from con3. */
    detached_text1 = get_detached_sigs(con3, con_md3);
    tt_assert(detached_text1);
    /* Try to parse it. */
    dsig1 = networkstatus_parse_detached_signatures(detached_text1, NULL);
    tt_assert(dsig1);

    /* Are parsed values as expected? */
    tt_int_op(dsig1->valid_after,OP_EQ, con3->valid_after);
    tt_int_op(dsig1->fresh_until,OP_EQ, con3->fresh_until);
    tt_int_op(dsig1->valid_until,OP_EQ, con3->valid_until);
    {
      common_digests_t *dsig_digests = strmap_get(dsig1->digests, "ns");
      tt_assert(dsig_digests);
      tt_mem_op(dsig_digests->d[DIGEST_SHA1], OP_EQ,
                con3->digests.d[DIGEST_SHA1], DIGEST_LEN);
      dsig_digests = strmap_get(dsig1->digests, "microdesc");
      tt_assert(dsig_digests);
      tt_mem_op(dsig_digests->d[DIGEST_SHA256],OP_EQ,
                 con_md3->digests.d[DIGEST_SHA256],
                 DIGEST256_LEN);
    }
    {
      smartlist_t *dsig_signatures = strmap_get(dsig1->signatures, "ns");
      tt_assert(dsig_signatures);
      tt_int_op(1,OP_EQ, smartlist_len(dsig_signatures));
      sig = smartlist_get(dsig_signatures, 0);
      tt_mem_op(sig->identity_digest,OP_EQ, cert1->cache_info.identity_digest,
                 DIGEST_LEN);
      tt_int_op(sig->alg,OP_EQ, DIGEST_SHA1);

      dsig_signatures = strmap_get(dsig1->signatures, "microdesc");
      tt_assert(dsig_signatures);
      tt_int_op(1,OP_EQ, smartlist_len(dsig_signatures));
      sig = smartlist_get(dsig_signatures, 0);
      tt_mem_op(sig->identity_digest,OP_EQ, cert1->cache_info.identity_digest,
                 DIGEST_LEN);
      tt_int_op(sig->alg,OP_EQ, DIGEST_SHA256);
    }

    /* Try adding it to con2. */
    detached_text2 = get_detached_sigs(con2,con_md2);
    tt_int_op(1,OP_EQ, networkstatus_add_detached_signatures(con2, dsig1,
                                                   "test", LOG_INFO, &msg));
    tor_free(detached_text2);
    tt_int_op(1,OP_EQ,
              networkstatus_add_detached_signatures(con_md2, dsig1, "test",
                                                     LOG_INFO, &msg));
    tor_free(detached_text2);
    detached_text2 = get_detached_sigs(con2,con_md2);
    //printf("\n<%s>\n", detached_text2);
    dsig2 = networkstatus_parse_detached_signatures(detached_text2, NULL);
    tt_assert(dsig2);
    /*
    printf("\n");
    SMARTLIST_FOREACH(dsig2->signatures, networkstatus_voter_info_t *, vi, {
        char hd[64];
        base16_encode(hd, sizeof(hd), vi->identity_digest, DIGEST_LEN);
        printf("%s\n", hd);
      });
    */
    tt_int_op(2,OP_EQ,
            smartlist_len((smartlist_t*)strmap_get(dsig2->signatures, "ns")));
    tt_int_op(2,OP_EQ,
            smartlist_len((smartlist_t*)strmap_get(dsig2->signatures,
                                                   "microdesc")));

    /* Try adding to con2 twice; verify that nothing changes. */
    tt_int_op(0,OP_EQ, networkstatus_add_detached_signatures(con2, dsig1,
                                               "test", LOG_INFO, &msg));

    /* Add to con. */
    tt_int_op(2,OP_EQ, networkstatus_add_detached_signatures(con, dsig2,
                                               "test", LOG_INFO, &msg));
    /* Check signatures */
    voter = smartlist_get(con->voters, 1);
    sig = smartlist_get(voter->sigs, 0);
    tt_assert(sig);
    tt_assert(!networkstatus_check_document_signature(con, sig, cert2));
    voter = smartlist_get(con->voters, 2);
    sig = smartlist_get(voter->sigs, 0);
    tt_assert(sig);
    tt_assert(!networkstatus_check_document_signature(con, sig, cert1));
  }

 done:
  tor_free(cp);
  smartlist_free(votes);
  tor_free(consensus_text);
  tor_free(consensus_text_md);

  networkstatus_vote_free(vote);
  networkstatus_vote_free(v1);
  networkstatus_vote_free(v2);
  networkstatus_vote_free(v3);
  networkstatus_vote_free(con);
  networkstatus_vote_free(con_md);
  crypto_pk_free(sign_skey_1);
  crypto_pk_free(sign_skey_2);
  crypto_pk_free(sign_skey_3);
  crypto_pk_free(sign_skey_leg1);
  authority_cert_free(cert1);
  authority_cert_free(cert2);
  authority_cert_free(cert3);

  tor_free(consensus_text2);
  tor_free(consensus_text3);
  tor_free(consensus_text_md2);
  tor_free(consensus_text_md3);
  tor_free(detached_text1);
  tor_free(detached_text2);

  networkstatus_vote_free(con2);
  networkstatus_vote_free(con3);
  networkstatus_vote_free(con_md2);
  networkstatus_vote_free(con_md3);
  ns_detached_signatures_free(dsig1);
  ns_detached_signatures_free(dsig2);
}

/** Run unit tests for generating and parsing V3 consensus networkstatus
 * documents. */
static void
test_dir_v3_networkstatus(void *arg)
{
  (void)arg;
  test_a_networkstatus(dir_common_gen_routerstatus_for_v3ns,
                       vote_tweaks_for_v3ns,
                       test_vrs_for_v3ns,
                       test_consensus_for_v3ns,
                       test_routerstatus_for_v3ns);
}

static void
test_dir_scale_bw(void *testdata)
{
  double v[8] = { 2.0/3,
                  7.0,
                  1.0,
                  3.0,
                  1.0/5,
                  1.0/7,
                  12.0,
                  24.0 };
  double vals_dbl[8];
  uint64_t vals_u64[8];
  uint64_t total;
  int i;

  (void) testdata;

  for (i=0; i<8; ++i)
    vals_dbl[i] = v[i];

  scale_array_elements_to_u64(vals_u64, vals_dbl, 8, &total);

  tt_int_op((int)total, OP_EQ, 48);
  total = 0;
  for (i=0; i<8; ++i) {
    total += vals_u64[i];
  }
  tt_assert(total >= (UINT64_C(1)<<60));
  tt_assert(total <= (UINT64_C(1)<<62));

  for (i=0; i<8; ++i) {
    /* vals[2].u64 is the scaled value of 1.0 */
    double ratio = ((double)vals_u64[i]) / vals_u64[2];
    tt_double_op(fabs(ratio - v[i]), OP_LT, .00001);
  }

  /* test handling of no entries */
  total = 1;
  scale_array_elements_to_u64(vals_u64, vals_dbl, 0, &total);
  tt_assert(total == 0);

  /* make sure we don't read the array when we have no entries
   * may require compiler flags to catch NULL dereferences */
  total = 1;
  scale_array_elements_to_u64(NULL, NULL, 0, &total);
  tt_assert(total == 0);

  scale_array_elements_to_u64(NULL, NULL, 0, NULL);

  /* test handling of zero totals */
  total = 1;
  vals_dbl[0] = 0.0;
  scale_array_elements_to_u64(vals_u64, vals_dbl, 1, &total);
  tt_assert(total == 0);
  tt_assert(vals_u64[0] == 0);

  vals_dbl[0] = 0.0;
  vals_dbl[1] = 0.0;
  scale_array_elements_to_u64(vals_u64, vals_dbl, 2, NULL);
  tt_assert(vals_u64[0] == 0);
  tt_assert(vals_u64[1] == 0);

 done:
  ;
}

static void
test_dir_random_weighted(void *testdata)
{
  int histogram[10];
  uint64_t vals[10] = {3,1,2,4,6,0,7,5,8,9}, total=0;
  uint64_t inp_u64[10];
  int i, choice;
  const int n = 50000;
  double max_sq_error;
  (void) testdata;

  /* Try a ten-element array with values from 0 through 10. The values are
   * in a scrambled order to make sure we don't depend on order. */
  memset(histogram,0,sizeof(histogram));
  for (i=0; i<10; ++i) {
    inp_u64[i] = vals[i];
    total += vals[i];
  }
  tt_u64_op(total, OP_EQ, 45);
  for (i=0; i<n; ++i) {
    choice = choose_array_element_by_weight(inp_u64, 10);
    tt_int_op(choice, OP_GE, 0);
    tt_int_op(choice, OP_LT, 10);
    histogram[choice]++;
  }

  /* Now see if we chose things about frequently enough. */
  max_sq_error = 0;
  for (i=0; i<10; ++i) {
    int expected = (int)(n*vals[i]/total);
    double frac_diff = 0, sq;
    TT_BLATHER(("  %d : %5d vs %5d\n", (int)vals[i], histogram[i], expected));
    if (expected)
      frac_diff = (histogram[i] - expected) / ((double)expected);
    else
      tt_int_op(histogram[i], OP_EQ, 0);

    sq = frac_diff * frac_diff;
    if (sq > max_sq_error)
      max_sq_error = sq;
  }
  /* It should almost always be much much less than this.  If you want to
   * figure out the odds, please feel free. */
  tt_double_op(max_sq_error, OP_LT, .05);

  /* Now try a singleton; do we choose it? */
  for (i = 0; i < 100; ++i) {
    choice = choose_array_element_by_weight(inp_u64, 1);
    tt_int_op(choice, OP_EQ, 0);
  }

  /* Now try an array of zeros.  We should choose randomly. */
  memset(histogram,0,sizeof(histogram));
  for (i = 0; i < 5; ++i)
    inp_u64[i] = 0;
  for (i = 0; i < n; ++i) {
    choice = choose_array_element_by_weight(inp_u64, 5);
    tt_int_op(choice, OP_GE, 0);
    tt_int_op(choice, OP_LT, 5);
    histogram[choice]++;
  }
  /* Now see if we chose things about frequently enough. */
  max_sq_error = 0;
  for (i=0; i<5; ++i) {
    int expected = n/5;
    double frac_diff = 0, sq;
    TT_BLATHER(("  %d : %5d vs %5d\n", (int)vals[i], histogram[i], expected));
    frac_diff = (histogram[i] - expected) / ((double)expected);
    sq = frac_diff * frac_diff;
    if (sq > max_sq_error)
      max_sq_error = sq;
  }
  /* It should almost always be much much less than this.  If you want to
   * figure out the odds, please feel free. */
  tt_double_op(max_sq_error, OP_LT, .05);
 done:
  ;
}

/* Function pointers for test_dir_clip_unmeasured_bw_kb() */

static uint32_t alternate_clip_bw = 0;

/**
 * Generate a routerstatus for clip_unmeasured_bw_kb test; based on the
 * v3_networkstatus ones.
 */
static vote_routerstatus_t *
gen_routerstatus_for_umbw(int idx, time_t now)
{
  vote_routerstatus_t *vrs = NULL;
  routerstatus_t *rs;
  tor_addr_t addr_ipv6;
  uint32_t max_unmeasured_bw_kb = (alternate_clip_bw > 0) ?
    alternate_clip_bw : DEFAULT_MAX_UNMEASURED_BW_KB;

  switch (idx) {
    case 0:
      /* Generate the first routerstatus. */
      vrs = tor_malloc_zero(sizeof(vote_routerstatus_t));
      rs = &vrs->status;
      vrs->version = tor_strdup("0.1.2.14");
      rs->published_on = now-1500;
      strlcpy(rs->nickname, "router2", sizeof(rs->nickname));
      memset(rs->identity_digest, 3, DIGEST_LEN);
      memset(rs->descriptor_digest, 78, DIGEST_LEN);
      rs->addr = 0x99008801;
      rs->or_port = 443;
      rs->dir_port = 8000;
      /* all flags but running and valid cleared */
      rs->is_flagged_running = 1;
      rs->is_valid = 1;
      /*
       * This one has measured bandwidth below the clip cutoff, and
       * so shouldn't be clipped; we'll have to test that it isn't
       * later.
       */
      vrs->has_measured_bw = 1;
      rs->has_bandwidth = 1;
      vrs->measured_bw_kb = rs->bandwidth_kb = max_unmeasured_bw_kb / 2;
      break;
    case 1:
      /* Generate the second routerstatus. */
      vrs = tor_malloc_zero(sizeof(vote_routerstatus_t));
      rs = &vrs->status;
      vrs->version = tor_strdup("0.2.0.5");
      rs->published_on = now-1000;
      strlcpy(rs->nickname, "router1", sizeof(rs->nickname));
      memset(rs->identity_digest, 5, DIGEST_LEN);
      memset(rs->descriptor_digest, 77, DIGEST_LEN);
      rs->addr = 0x99009901;
      rs->or_port = 443;
      rs->dir_port = 0;
      tor_addr_parse(&addr_ipv6, "[1:2:3::4]");
      tor_addr_copy(&rs->ipv6_addr, &addr_ipv6);
      rs->ipv6_orport = 4711;
      rs->is_exit = rs->is_stable = rs->is_fast = rs->is_flagged_running =
        rs->is_valid = rs->is_possible_guard = 1;
      /*
       * This one has measured bandwidth above the clip cutoff, and
       * so shouldn't be clipped; we'll have to test that it isn't
       * later.
       */
      vrs->has_measured_bw = 1;
      rs->has_bandwidth = 1;
      vrs->measured_bw_kb = rs->bandwidth_kb = 2 * max_unmeasured_bw_kb;
      break;
    case 2:
      /* Generate the third routerstatus. */
      vrs = tor_malloc_zero(sizeof(vote_routerstatus_t));
      rs = &vrs->status;
      vrs->version = tor_strdup("0.1.0.3");
      rs->published_on = now-1000;
      strlcpy(rs->nickname, "router3", sizeof(rs->nickname));
      memset(rs->identity_digest, 0x33, DIGEST_LEN);
      memset(rs->descriptor_digest, 79, DIGEST_LEN);
      rs->addr = 0xAA009901;
      rs->or_port = 400;
      rs->dir_port = 9999;
      rs->is_authority = rs->is_exit = rs->is_stable = rs->is_fast =
        rs->is_flagged_running = rs->is_valid =
        rs->is_possible_guard = 1;
      /*
       * This one has unmeasured bandwidth above the clip cutoff, and
       * so should be clipped; we'll have to test that it isn't
       * later.
       */
      vrs->has_measured_bw = 0;
      rs->has_bandwidth = 1;
      vrs->measured_bw_kb = 0;
      rs->bandwidth_kb = 2 * max_unmeasured_bw_kb;
      break;
    case 3:
      /* Generate a fourth routerstatus that is not running. */
      vrs = tor_malloc_zero(sizeof(vote_routerstatus_t));
      rs = &vrs->status;
      vrs->version = tor_strdup("0.1.6.3");
      rs->published_on = now-1000;
      strlcpy(rs->nickname, "router4", sizeof(rs->nickname));
      memset(rs->identity_digest, 0x34, DIGEST_LEN);
      memset(rs->descriptor_digest, 47, DIGEST_LEN);
      rs->addr = 0xC0000203;
      rs->or_port = 500;
      rs->dir_port = 1999;
      /* all flags but running and valid cleared */
      rs->is_flagged_running = 1;
      rs->is_valid = 1;
      /*
       * This one has unmeasured bandwidth below the clip cutoff, and
       * so shouldn't be clipped; we'll have to test that it isn't
       * later.
       */
      vrs->has_measured_bw = 0;
      rs->has_bandwidth = 1;
      vrs->measured_bw_kb = 0;
      rs->bandwidth_kb = max_unmeasured_bw_kb / 2;
      break;
    case 4:
      /* No more for this test; return NULL */
      vrs = NULL;
      break;
    default:
      /* Shouldn't happen */
      tt_abort();
  }
  if (vrs) {
    vrs->microdesc = tor_malloc_zero(sizeof(vote_microdesc_hash_t));
    tor_asprintf(&vrs->microdesc->microdesc_hash_line,
                 "m 25,26,27,28 "
                 "sha256=xyzajkldsdsajdadlsdjaslsdksdjlsdjsdaskdaaa%d\n",
                 idx);
  }

 done:
  return vrs;
}

/** Apply tweaks to the vote list for each voter; for the umbw test this is
 * just adding the right consensus methods to let clipping happen */
static int
vote_tweaks_for_umbw(networkstatus_t *v, int voter, time_t now)
{
  char *maxbw_param = NULL;
  int rv = 0;

  tt_assert(v);
  (void)voter;
  (void)now;

  tt_assert(v->supported_methods);
  SMARTLIST_FOREACH(v->supported_methods, char *, c, tor_free(c));
  smartlist_clear(v->supported_methods);
  /* Method 17 is MIN_METHOD_TO_CLIP_UNMEASURED_BW_KB */
  smartlist_split_string(v->supported_methods,
                         "25 26 27 28",
                         NULL, 0, -1);
  /* If we're using a non-default clip bandwidth, add it to net_params */
  if (alternate_clip_bw > 0) {
    tor_asprintf(&maxbw_param, "maxunmeasuredbw=%u", alternate_clip_bw);
    tt_assert(maxbw_param);
    if (maxbw_param) {
      smartlist_add(v->net_params, maxbw_param);
      rv = 1;
    }
  }

 done:
  return rv;
}

/**
 * Test a parsed vote_routerstatus_t for umbw test.
 */
static void
test_vrs_for_umbw(vote_routerstatus_t *vrs, int voter, time_t now)
{
  routerstatus_t *rs;
  tor_addr_t addr_ipv6;
  uint32_t max_unmeasured_bw_kb = (alternate_clip_bw > 0) ?
    alternate_clip_bw : DEFAULT_MAX_UNMEASURED_BW_KB;

  (void)voter;
  tt_assert(vrs);
  rs = &(vrs->status);
  tt_assert(rs);

  /* Split out by digests to test */
  if (tor_memeq(rs->identity_digest,
                "\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3"
                "\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3",
                DIGEST_LEN)) {
    /*
     * Check the first routerstatus - measured bandwidth below the clip
     * cutoff.
     */
    tt_str_op(vrs->version,OP_EQ, "0.1.2.14");
    tt_int_op(rs->published_on,OP_EQ, now-1500);
    tt_str_op(rs->nickname,OP_EQ, "router2");
    tt_mem_op(rs->identity_digest,OP_EQ,
               "\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3"
               "\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3",
               DIGEST_LEN);
    tt_mem_op(rs->descriptor_digest,OP_EQ, "NNNNNNNNNNNNNNNNNNNN", DIGEST_LEN);
    tt_int_op(rs->addr,OP_EQ, 0x99008801);
    tt_int_op(rs->or_port,OP_EQ, 443);
    tt_int_op(rs->dir_port,OP_EQ, 8000);
    tt_assert(rs->has_bandwidth);
    tt_assert(vrs->has_measured_bw);
    tt_int_op(rs->bandwidth_kb,OP_EQ, max_unmeasured_bw_kb / 2);
    tt_int_op(vrs->measured_bw_kb,OP_EQ, max_unmeasured_bw_kb / 2);
  } else if (tor_memeq(rs->identity_digest,
                       "\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5"
                       "\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5",
                       DIGEST_LEN)) {

    /*
     * Check the second routerstatus - measured bandwidth above the clip
     * cutoff.
     */
    tt_str_op(vrs->version,OP_EQ, "0.2.0.5");
    tt_int_op(rs->published_on,OP_EQ, now-1000);
    tt_str_op(rs->nickname,OP_EQ, "router1");
    tt_mem_op(rs->identity_digest,OP_EQ,
               "\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5"
               "\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5",
               DIGEST_LEN);
    tt_mem_op(rs->descriptor_digest,OP_EQ, "MMMMMMMMMMMMMMMMMMMM", DIGEST_LEN);
    tt_int_op(rs->addr,OP_EQ, 0x99009901);
    tt_int_op(rs->or_port,OP_EQ, 443);
    tt_int_op(rs->dir_port,OP_EQ, 0);
    tor_addr_parse(&addr_ipv6, "[1:2:3::4]");
    tt_assert(tor_addr_eq(&rs->ipv6_addr, &addr_ipv6));
    tt_int_op(rs->ipv6_orport,OP_EQ, 4711);
    tt_assert(rs->has_bandwidth);
    tt_assert(vrs->has_measured_bw);
    tt_int_op(rs->bandwidth_kb,OP_EQ, max_unmeasured_bw_kb * 2);
    tt_int_op(vrs->measured_bw_kb,OP_EQ, max_unmeasured_bw_kb * 2);
  } else if (tor_memeq(rs->identity_digest,
                       "\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33"
                       "\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33",
                       DIGEST_LEN)) {
    /*
     * Check the third routerstatus - unmeasured bandwidth above the clip
     * cutoff; this one should be clipped later on in the consensus, but
     * appears unclipped in the vote.
     */
    tt_assert(rs->has_bandwidth);
    tt_assert(!(vrs->has_measured_bw));
    tt_int_op(rs->bandwidth_kb,OP_EQ, max_unmeasured_bw_kb * 2);
    tt_int_op(vrs->measured_bw_kb,OP_EQ, 0);
  } else if (tor_memeq(rs->identity_digest,
                       "\x34\x34\x34\x34\x34\x34\x34\x34\x34\x34"
                       "\x34\x34\x34\x34\x34\x34\x34\x34\x34\x34",
                       DIGEST_LEN)) {
    /*
     * Check the fourth routerstatus - unmeasured bandwidth below the clip
     * cutoff; this one should not be clipped.
     */
    tt_assert(rs->has_bandwidth);
    tt_assert(!(vrs->has_measured_bw));
    tt_int_op(rs->bandwidth_kb,OP_EQ, max_unmeasured_bw_kb / 2);
    tt_int_op(vrs->measured_bw_kb,OP_EQ, 0);
  } else {
    tt_abort();
  }

 done:
  return;
}

/**
 * Test a consensus for v3_networkstatus_test
 */
static void
test_consensus_for_umbw(networkstatus_t *con, time_t now)
{
  (void)now;

  tt_assert(con);
  tt_ptr_op(con->cert, OP_EQ, NULL);
  // tt_assert(con->consensus_method >= MIN_METHOD_TO_CLIP_UNMEASURED_BW_KB);
  tt_int_op(con->consensus_method, OP_GE, 16);
  tt_int_op(4,OP_EQ, smartlist_len(con->routerstatus_list));
  /* There should be four listed routers; all voters saw the same in this */

 done:
  return;
}

/**
 * Test a router list entry for umbw test
 */
static void
test_routerstatus_for_umbw(routerstatus_t *rs, time_t now)
{
  tor_addr_t addr_ipv6;
  uint32_t max_unmeasured_bw_kb = (alternate_clip_bw > 0) ?
    alternate_clip_bw : DEFAULT_MAX_UNMEASURED_BW_KB;

  tt_assert(rs);

  /* There should be four listed routers, as constructed above */
  if (tor_memeq(rs->identity_digest,
                "\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3"
                "\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3",
                DIGEST_LEN)) {
    tt_mem_op(rs->identity_digest,OP_EQ,
               "\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3"
               "\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3",
               DIGEST_LEN);
    tt_mem_op(rs->descriptor_digest,OP_EQ, "NNNNNNNNNNNNNNNNNNNN", DIGEST_LEN);
    tt_assert(!rs->is_authority);
    tt_assert(!rs->is_exit);
    tt_assert(!rs->is_fast);
    tt_assert(!rs->is_possible_guard);
    tt_assert(!rs->is_stable);
    /* (If it wasn't running and valid it wouldn't be here) */
    tt_assert(rs->is_flagged_running);
    tt_assert(rs->is_valid);
    tt_assert(!rs->is_named);
    /* This one should have measured bandwidth below the clip cutoff */
    tt_assert(rs->has_bandwidth);
    tt_int_op(rs->bandwidth_kb,OP_EQ, max_unmeasured_bw_kb / 2);
    tt_assert(!(rs->bw_is_unmeasured));
  } else if (tor_memeq(rs->identity_digest,
                       "\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5"
                       "\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5",
                       DIGEST_LEN)) {
    /* This one showed up in 3 digests. Twice with ID 'M', once with 'Z'.  */
    tt_mem_op(rs->identity_digest,OP_EQ,
               "\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5"
               "\x5\x5\x5\x5\x5\x5\x5\x5\x5\x5",
               DIGEST_LEN);
    tt_str_op(rs->nickname,OP_EQ, "router1");
    tt_mem_op(rs->descriptor_digest,OP_EQ, "MMMMMMMMMMMMMMMMMMMM", DIGEST_LEN);
    tt_int_op(rs->published_on,OP_EQ, now-1000);
    tt_int_op(rs->addr,OP_EQ, 0x99009901);
    tt_int_op(rs->or_port,OP_EQ, 443);
    tt_int_op(rs->dir_port,OP_EQ, 0);
    tor_addr_parse(&addr_ipv6, "[1:2:3::4]");
    tt_assert(tor_addr_eq(&rs->ipv6_addr, &addr_ipv6));
    tt_int_op(rs->ipv6_orport,OP_EQ, 4711);
    tt_assert(!rs->is_authority);
    tt_assert(rs->is_exit);
    tt_assert(rs->is_fast);
    tt_assert(rs->is_possible_guard);
    tt_assert(rs->is_stable);
    tt_assert(rs->is_flagged_running);
    tt_assert(rs->is_valid);
    tt_assert(!rs->is_named);
    /* This one should have measured bandwidth above the clip cutoff */
    tt_assert(rs->has_bandwidth);
    tt_int_op(rs->bandwidth_kb,OP_EQ, max_unmeasured_bw_kb * 2);
    tt_assert(!(rs->bw_is_unmeasured));
  } else if (tor_memeq(rs->identity_digest,
                "\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33"
                "\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33",
                DIGEST_LEN)) {
    /*
     * This one should have unmeasured bandwidth above the clip cutoff,
     * and so should be clipped
     */
    tt_assert(rs->has_bandwidth);
    tt_int_op(rs->bandwidth_kb,OP_EQ, max_unmeasured_bw_kb);
    tt_assert(rs->bw_is_unmeasured);
  } else if (tor_memeq(rs->identity_digest,
                "\x34\x34\x34\x34\x34\x34\x34\x34\x34\x34"
                "\x34\x34\x34\x34\x34\x34\x34\x34\x34\x34",
                DIGEST_LEN)) {
    /*
     * This one should have unmeasured bandwidth below the clip cutoff,
     * and so should not be clipped
     */
    tt_assert(rs->has_bandwidth);
    tt_int_op(rs->bandwidth_kb,OP_EQ, max_unmeasured_bw_kb / 2);
    tt_assert(rs->bw_is_unmeasured);
  } else {
    /* Weren't expecting this... */
    tt_abort();
  }

 done:
  return;
}

/**
 * Compute a consensus involving clipping unmeasured bandwidth with consensus
 * method 17; this uses the same test_a_networkstatus() function that the
 * v3_networkstatus test uses.
 */

static void
test_dir_clip_unmeasured_bw_kb(void *arg)
{
  /* Run the test with the default clip bandwidth */
  (void)arg;
  alternate_clip_bw = 0;
  test_a_networkstatus(gen_routerstatus_for_umbw,
                       vote_tweaks_for_umbw,
                       test_vrs_for_umbw,
                       test_consensus_for_umbw,
                       test_routerstatus_for_umbw);
}

/**
 * This version of test_dir_clip_unmeasured_bw_kb() uses a non-default choice
 * of clip bandwidth.
 */

static void
test_dir_clip_unmeasured_bw_kb_alt(void *arg)
{
  /*
   * Try a different one; this value is chosen so that the below-the-cutoff
   * unmeasured nodes the test uses, at alternate_clip_bw / 2, will be above
   * DEFAULT_MAX_UNMEASURED_BW_KB and if the consensus incorrectly uses that
   * cutoff it will fail the test.
   */
  (void)arg;
  alternate_clip_bw = 3 * DEFAULT_MAX_UNMEASURED_BW_KB;
  test_a_networkstatus(gen_routerstatus_for_umbw,
                       vote_tweaks_for_umbw,
                       test_vrs_for_umbw,
                       test_consensus_for_umbw,
                       test_routerstatus_for_umbw);
}

static void
test_dir_fmt_control_ns(void *arg)
{
  char *s = NULL;
  routerstatus_t rs;
  (void)arg;

  memset(&rs, 0, sizeof(rs));
  rs.published_on = 1364925198;
  strlcpy(rs.nickname, "TetsuoMilk", sizeof(rs.nickname));
  memcpy(rs.identity_digest, "Stately, plump Buck ", DIGEST_LEN);
  memcpy(rs.descriptor_digest, "Mulligan came up fro", DIGEST_LEN);
  rs.addr = 0x20304050;
  rs.or_port = 9001;
  rs.dir_port = 9002;
  rs.is_exit = 1;
  rs.is_fast = 1;
  rs.is_flagged_running = 1;
  rs.has_bandwidth = 1;
  rs.is_v2_dir = 1;
  rs.bandwidth_kb = 1000;

  s = networkstatus_getinfo_helper_single(&rs);
  tt_assert(s);
  tt_str_op(s, OP_EQ,
            "r TetsuoMilk U3RhdGVseSwgcGx1bXAgQnVjayA "
               "TXVsbGlnYW4gY2FtZSB1cCBmcm8 2013-04-02 17:53:18 "
               "32.48.64.80 9001 9002\n"
            "s Exit Fast Running V2Dir\n"
            "w Bandwidth=1000\n");

 done:
  tor_free(s);
}

static int mock_get_options_calls = 0;
static or_options_t *mock_options = NULL;

static void
reset_options(or_options_t *options, int *get_options_calls)
{
  memset(options, 0, sizeof(or_options_t));
  options->TestingTorNetwork = 1;

  *get_options_calls = 0;
}

static const or_options_t *
mock_get_options(void)
{
  ++mock_get_options_calls;
  tor_assert(mock_options);
  return mock_options;
}

/**
 * Test dirauth_get_b64_digest_bw_file.
 * This function should be near the other bwauth functions, but it needs
 * mock_get_options, that is only defined here.
 */

static void
test_dir_bwauth_bw_file_digest256(void *arg)
{
  (void)arg;
  const char *content =
    "1541171221\n"
    "node_id=$68A483E05A2ABDCA6DA5A3EF8DB5177638A27F80 "
    "master_key_ed25519=YaqV4vbvPYKucElk297eVdNArDz9HtIwUoIeo0+cVIpQ "
    "bw=760 nick=Test time=2018-05-08T16:13:26\n";

  char *fname = tor_strdup(get_fname("V3BandwidthsFile"));
  /* Initialize to a wrong digest. */
  uint8_t digest[DIGEST256_LEN] = "01234567890123456789abcdefghijkl";

  /* Digest of an empty string. Initialize to a wrong digest. */
  char digest_empty_str[DIGEST256_LEN] = "01234567890123456789abcdefghijkl";
  crypto_digest256(digest_empty_str, "", 0, DIGEST_SHA256);

  /* Digest of the content. Initialize to a wrong digest. */
  char digest_expected[DIGEST256_LEN] = "01234567890123456789abcdefghijkl";
  crypto_digest256(digest_expected, content, strlen(content), DIGEST_SHA256);

  /* When the bandwidth file can not be found. */
  tt_int_op(-1, OP_EQ,
            dirserv_read_measured_bandwidths(fname,
                                             NULL, NULL, digest));
  tt_mem_op(digest, OP_EQ, digest_empty_str, DIGEST256_LEN);

  /* When there is a timestamp but it is too old. */
  write_str_to_file(fname, content, 0);
  tt_int_op(-1, OP_EQ,
            dirserv_read_measured_bandwidths(fname,
                                             NULL, NULL, digest));
  /* The digest will be correct. */
  tt_mem_op(digest, OP_EQ, digest_expected, DIGEST256_LEN);

  update_approx_time(1541171221);

  /* When there is a bandwidth file and it can be read. */
  tt_int_op(0, OP_EQ,
            dirserv_read_measured_bandwidths(fname,
                                             NULL, NULL, digest));
  tt_mem_op(digest, OP_EQ, digest_expected, DIGEST256_LEN);

 done:
  unlink(fname);
  tor_free(fname);
  update_approx_time(time(NULL));
}

static void
reset_routerstatus(routerstatus_t *rs,
                   const char *hex_identity_digest,
                   uint32_t ipv4_addr)
{
  memset(rs, 0, sizeof(routerstatus_t));
  base16_decode(rs->identity_digest, sizeof(rs->identity_digest),
                hex_identity_digest, HEX_DIGEST_LEN);
  /* A zero address matches everything, so the address needs to be set.
   * But the specific value is irrelevant. */
  rs->addr = ipv4_addr;
}

#define ROUTER_A_ID_STR    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
#define ROUTER_A_IPV4      0xAA008801
#define ROUTER_B_ID_STR    "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
#define ROUTER_B_IPV4      0xBB008801

#define ROUTERSET_ALL_STR  "*"
#define ROUTERSET_A_STR    ROUTER_A_ID_STR
#define ROUTERSET_NONE_STR ""

/*
 * Test that dirserv_set_routerstatus_testing sets router flags correctly
 * Using "*"  sets flags on A and B
 * Using "A"  sets flags on A
 * Using ""   sets flags on Neither
 * If the router is not included:
 *   - if *Strict is set, the flag is set to 0,
 *   - otherwise, the flag is not modified. */
static void
test_dir_dirserv_set_routerstatus_testing(void *arg)
{
  (void)arg;

  /* Init options */
  mock_options = tor_malloc(sizeof(or_options_t));
  reset_options(mock_options, &mock_get_options_calls);

  MOCK(get_options, mock_get_options);

  /* Init routersets */
  routerset_t *routerset_all  = routerset_new();
  routerset_parse(routerset_all,  ROUTERSET_ALL_STR,  "All routers");

  routerset_t *routerset_a    = routerset_new();
  routerset_parse(routerset_a,    ROUTERSET_A_STR,    "Router A only");

  routerset_t *routerset_none = routerset_new();
  /* Routersets are empty when provided by routerset_new(),
   * so this is not strictly necessary */
  routerset_parse(routerset_none, ROUTERSET_NONE_STR, "No routers");

  /* Init routerstatuses */
  routerstatus_t *rs_a = tor_malloc(sizeof(routerstatus_t));
  reset_routerstatus(rs_a, ROUTER_A_ID_STR, ROUTER_A_IPV4);

  routerstatus_t *rs_b = tor_malloc(sizeof(routerstatus_t));
  reset_routerstatus(rs_b, ROUTER_B_ID_STR, ROUTER_B_IPV4);

  /* Sanity check that routersets correspond to routerstatuses.
   * Return values are {2, 3, 4} */

  /* We want 3 ("*" means match all addresses) */
  tt_int_op(routerset_contains_routerstatus(routerset_all, rs_a, 0), OP_EQ, 3);
  tt_int_op(routerset_contains_routerstatus(routerset_all, rs_b, 0), OP_EQ, 3);

  /* We want 4 (match id_digest [or nickname]) */
  tt_int_op(routerset_contains_routerstatus(routerset_a, rs_a, 0), OP_EQ, 4);
  tt_int_op(routerset_contains_routerstatus(routerset_a, rs_b, 0), OP_EQ, 0);

  tt_int_op(routerset_contains_routerstatus(routerset_none, rs_a, 0), OP_EQ,
            0);
  tt_int_op(routerset_contains_routerstatus(routerset_none, rs_b, 0), OP_EQ,
            0);

  /* Check that "*" sets flags on all routers: Exit
   * Check the flags aren't being confused with each other */
  reset_options(mock_options, &mock_get_options_calls);
  reset_routerstatus(rs_a, ROUTER_A_ID_STR, ROUTER_A_IPV4);
  reset_routerstatus(rs_b, ROUTER_B_ID_STR, ROUTER_B_IPV4);

  mock_options->TestingDirAuthVoteExit = routerset_all;
  mock_options->TestingDirAuthVoteExitIsStrict = 0;

  dirserv_set_routerstatus_testing(rs_a);
  tt_int_op(mock_get_options_calls, OP_EQ, 1);
  dirserv_set_routerstatus_testing(rs_b);
  tt_int_op(mock_get_options_calls, OP_EQ, 2);

  tt_uint_op(rs_a->is_exit, OP_EQ, 1);
  tt_uint_op(rs_b->is_exit, OP_EQ, 1);
  /* Be paranoid - check no other flags are set */
  tt_uint_op(rs_a->is_possible_guard, OP_EQ, 0);
  tt_uint_op(rs_b->is_possible_guard, OP_EQ, 0);
  tt_uint_op(rs_a->is_hs_dir, OP_EQ, 0);
  tt_uint_op(rs_b->is_hs_dir, OP_EQ, 0);

  /* Check that "*" sets flags on all routers: Guard & HSDir
   * Cover the remaining flags in one test */
  reset_options(mock_options, &mock_get_options_calls);
  reset_routerstatus(rs_a, ROUTER_A_ID_STR, ROUTER_A_IPV4);
  reset_routerstatus(rs_b, ROUTER_B_ID_STR, ROUTER_B_IPV4);

  mock_options->TestingDirAuthVoteGuard = routerset_all;
  mock_options->TestingDirAuthVoteGuardIsStrict = 0;
  mock_options->TestingDirAuthVoteHSDir = routerset_all;
  mock_options->TestingDirAuthVoteHSDirIsStrict = 0;

  dirserv_set_routerstatus_testing(rs_a);
  tt_int_op(mock_get_options_calls, OP_EQ, 1);
  dirserv_set_routerstatus_testing(rs_b);
  tt_int_op(mock_get_options_calls, OP_EQ, 2);

  tt_uint_op(rs_a->is_possible_guard, OP_EQ, 1);
  tt_uint_op(rs_b->is_possible_guard, OP_EQ, 1);
  tt_uint_op(rs_a->is_hs_dir, OP_EQ, 1);
  tt_uint_op(rs_b->is_hs_dir, OP_EQ, 1);
  /* Be paranoid - check exit isn't set */
  tt_uint_op(rs_a->is_exit, OP_EQ, 0);
  tt_uint_op(rs_b->is_exit, OP_EQ, 0);

  /* Check routerset A sets all flags on router A,
   * but leaves router B unmodified */
  reset_options(mock_options, &mock_get_options_calls);
  reset_routerstatus(rs_a, ROUTER_A_ID_STR, ROUTER_A_IPV4);
  reset_routerstatus(rs_b, ROUTER_B_ID_STR, ROUTER_B_IPV4);

  mock_options->TestingDirAuthVoteExit = routerset_a;
  mock_options->TestingDirAuthVoteExitIsStrict = 0;
  mock_options->TestingDirAuthVoteGuard = routerset_a;
  mock_options->TestingDirAuthVoteGuardIsStrict = 0;
  mock_options->TestingDirAuthVoteHSDir = routerset_a;
  mock_options->TestingDirAuthVoteHSDirIsStrict = 0;

  dirserv_set_routerstatus_testing(rs_a);
  tt_int_op(mock_get_options_calls, OP_EQ, 1);
  dirserv_set_routerstatus_testing(rs_b);
  tt_int_op(mock_get_options_calls, OP_EQ, 2);

  tt_uint_op(rs_a->is_exit, OP_EQ, 1);
  tt_uint_op(rs_b->is_exit, OP_EQ, 0);
  tt_uint_op(rs_a->is_possible_guard, OP_EQ, 1);
  tt_uint_op(rs_b->is_possible_guard, OP_EQ, 0);
  tt_uint_op(rs_a->is_hs_dir, OP_EQ, 1);
  tt_uint_op(rs_b->is_hs_dir, OP_EQ, 0);

  /* Check routerset A unsets all flags on router B when Strict is set */
  reset_options(mock_options, &mock_get_options_calls);
  reset_routerstatus(rs_b, ROUTER_B_ID_STR, ROUTER_B_IPV4);

  mock_options->TestingDirAuthVoteExit = routerset_a;
  mock_options->TestingDirAuthVoteExitIsStrict = 1;
  mock_options->TestingDirAuthVoteGuard = routerset_a;
  mock_options->TestingDirAuthVoteGuardIsStrict = 1;
  mock_options->TestingDirAuthVoteHSDir = routerset_a;
  mock_options->TestingDirAuthVoteHSDirIsStrict = 1;

  rs_b->is_exit = 1;
  rs_b->is_possible_guard = 1;
  rs_b->is_hs_dir = 1;

  dirserv_set_routerstatus_testing(rs_b);
  tt_int_op(mock_get_options_calls, OP_EQ, 1);

  tt_uint_op(rs_b->is_exit, OP_EQ, 0);
  tt_uint_op(rs_b->is_possible_guard, OP_EQ, 0);
  tt_uint_op(rs_b->is_hs_dir, OP_EQ, 0);

  /* Check routerset A doesn't modify flags on router B without Strict set */
  reset_options(mock_options, &mock_get_options_calls);
  reset_routerstatus(rs_b, ROUTER_B_ID_STR, ROUTER_B_IPV4);

  mock_options->TestingDirAuthVoteExit = routerset_a;
  mock_options->TestingDirAuthVoteExitIsStrict = 0;
  mock_options->TestingDirAuthVoteGuard = routerset_a;
  mock_options->TestingDirAuthVoteGuardIsStrict = 0;
  mock_options->TestingDirAuthVoteHSDir = routerset_a;
  mock_options->TestingDirAuthVoteHSDirIsStrict = 0;

  rs_b->is_exit = 1;
  rs_b->is_possible_guard = 1;
  rs_b->is_hs_dir = 1;

  dirserv_set_routerstatus_testing(rs_b);
  tt_int_op(mock_get_options_calls, OP_EQ, 1);

  tt_uint_op(rs_b->is_exit, OP_EQ, 1);
  tt_uint_op(rs_b->is_possible_guard, OP_EQ, 1);
  tt_uint_op(rs_b->is_hs_dir, OP_EQ, 1);

  /* Check the empty routerset zeroes all flags
   * on routers A & B with Strict set */
  reset_options(mock_options, &mock_get_options_calls);
  reset_routerstatus(rs_b, ROUTER_B_ID_STR, ROUTER_B_IPV4);

  mock_options->TestingDirAuthVoteExit = routerset_none;
  mock_options->TestingDirAuthVoteExitIsStrict = 1;
  mock_options->TestingDirAuthVoteGuard = routerset_none;
  mock_options->TestingDirAuthVoteGuardIsStrict = 1;
  mock_options->TestingDirAuthVoteHSDir = routerset_none;
  mock_options->TestingDirAuthVoteHSDirIsStrict = 1;

  rs_b->is_exit = 1;
  rs_b->is_possible_guard = 1;
  rs_b->is_hs_dir = 1;

  dirserv_set_routerstatus_testing(rs_b);
  tt_int_op(mock_get_options_calls, OP_EQ, 1);

  tt_uint_op(rs_b->is_exit, OP_EQ, 0);
  tt_uint_op(rs_b->is_possible_guard, OP_EQ, 0);
  tt_uint_op(rs_b->is_hs_dir, OP_EQ, 0);

  /* Check the empty routerset doesn't modify any flags
   * on A or B without Strict set */
  reset_options(mock_options, &mock_get_options_calls);
  reset_routerstatus(rs_a, ROUTER_A_ID_STR, ROUTER_A_IPV4);
  reset_routerstatus(rs_b, ROUTER_B_ID_STR, ROUTER_B_IPV4);

  mock_options->TestingDirAuthVoteExit = routerset_none;
  mock_options->TestingDirAuthVoteExitIsStrict = 0;
  mock_options->TestingDirAuthVoteGuard = routerset_none;
  mock_options->TestingDirAuthVoteGuardIsStrict = 0;
  mock_options->TestingDirAuthVoteHSDir = routerset_none;
  mock_options->TestingDirAuthVoteHSDirIsStrict = 0;

  rs_b->is_exit = 1;
  rs_b->is_possible_guard = 1;
  rs_b->is_hs_dir = 1;

  dirserv_set_routerstatus_testing(rs_a);
  tt_int_op(mock_get_options_calls, OP_EQ, 1);
  dirserv_set_routerstatus_testing(rs_b);
  tt_int_op(mock_get_options_calls, OP_EQ, 2);

  tt_uint_op(rs_a->is_exit, OP_EQ, 0);
  tt_uint_op(rs_a->is_possible_guard, OP_EQ, 0);
  tt_uint_op(rs_a->is_hs_dir, OP_EQ, 0);
  tt_uint_op(rs_b->is_exit, OP_EQ, 1);
  tt_uint_op(rs_b->is_possible_guard, OP_EQ, 1);
  tt_uint_op(rs_b->is_hs_dir, OP_EQ, 1);

 done:
  tor_free(mock_options);
  mock_options = NULL;

  UNMOCK(get_options);

  routerset_free(routerset_all);
  routerset_free(routerset_a);
  routerset_free(routerset_none);

  tor_free(rs_a);
  tor_free(rs_b);
}

static void
test_dir_http_handling(void *args)
{
  char *url = NULL;
  (void)args;

  /* Parse http url tests: */
  /* Good headers */
  tt_int_op(parse_http_url("GET /tor/a/b/c.txt HTTP/1.1\r\n"
                           "Host: example.com\r\n"
                           "User-Agent: Mozilla/5.0 (Windows;"
                           " U; Windows NT 6.1; en-US; rv:1.9.1.5)\r\n",
                           &url),OP_EQ, 0);
  tt_str_op(url,OP_EQ, "/tor/a/b/c.txt");
  tor_free(url);

  tt_int_op(parse_http_url("GET /tor/a/b/c.txt HTTP/1.0\r\n", &url),OP_EQ, 0);
  tt_str_op(url,OP_EQ, "/tor/a/b/c.txt");
  tor_free(url);

  tt_int_op(parse_http_url("GET /tor/a/b/c.txt HTTP/1.600\r\n", &url),
            OP_EQ, 0);
  tt_str_op(url,OP_EQ, "/tor/a/b/c.txt");
  tor_free(url);

  /* Should prepend '/tor/' to url if required */
  tt_int_op(parse_http_url("GET /a/b/c.txt HTTP/1.1\r\n"
                           "Host: example.com\r\n"
                           "User-Agent: Mozilla/5.0 (Windows;"
                           " U; Windows NT 6.1; en-US; rv:1.9.1.5)\r\n",
                           &url),OP_EQ, 0);
  tt_str_op(url,OP_EQ, "/tor/a/b/c.txt");
  tor_free(url);

  /* Bad headers -- no HTTP/1.x*/
  tt_int_op(parse_http_url("GET /a/b/c.txt\r\n"
                           "Host: example.com\r\n"
                           "User-Agent: Mozilla/5.0 (Windows;"
                           " U; Windows NT 6.1; en-US; rv:1.9.1.5)\r\n",
                           &url),OP_EQ, -1);
  tt_ptr_op(url, OP_EQ, NULL);

  /* Bad headers */
  tt_int_op(parse_http_url("GET /a/b/c.txt\r\n"
                           "Host: example.com\r\n"
                           "User-Agent: Mozilla/5.0 (Windows;"
                           " U; Windows NT 6.1; en-US; rv:1.9.1.5)\r\n",
                           &url),OP_EQ, -1);
  tt_ptr_op(url, OP_EQ, NULL);

  tt_int_op(parse_http_url("GET /tor/a/b/c.txt", &url),OP_EQ, -1);
  tt_ptr_op(url, OP_EQ, NULL);

  tt_int_op(parse_http_url("GET /tor/a/b/c.txt HTTP/1.1", &url),OP_EQ, -1);
  tt_ptr_op(url, OP_EQ, NULL);

  tt_int_op(parse_http_url("GET /tor/a/b/c.txt HTTP/1.1x\r\n", &url),
            OP_EQ, -1);
  tt_ptr_op(url, OP_EQ, NULL);

  tt_int_op(parse_http_url("GET /tor/a/b/c.txt HTTP/1.", &url),OP_EQ, -1);
  tt_ptr_op(url, OP_EQ, NULL);

  tt_int_op(parse_http_url("GET /tor/a/b/c.txt HTTP/1.\r", &url),OP_EQ, -1);
  tt_ptr_op(url, OP_EQ, NULL);

 done:
  tor_free(url);
}

static void
test_dir_purpose_needs_anonymity_returns_true_by_default(void *arg)
{
  (void)arg;

  tor_capture_bugs_(1);
  setup_full_capture_of_logs(LOG_WARN);
  tt_int_op(1, OP_EQ, purpose_needs_anonymity(0, 0, NULL));
  tt_int_op(1, OP_EQ, smartlist_len(tor_get_captured_bug_log_()));
  expect_single_log_msg_containing("Called with dir_purpose=0");

  tor_end_capture_bugs_();
 done:
  tor_end_capture_bugs_();
  teardown_capture_of_logs();
}

static void
test_dir_purpose_needs_anonymity_returns_true_for_bridges(void *arg)
{
  (void)arg;

  tt_int_op(1, OP_EQ, purpose_needs_anonymity(0, ROUTER_PURPOSE_BRIDGE, NULL));
  tt_int_op(1, OP_EQ, purpose_needs_anonymity(0, ROUTER_PURPOSE_BRIDGE,
                                           "foobar"));
  tt_int_op(1, OP_EQ,
            purpose_needs_anonymity(DIR_PURPOSE_HAS_FETCHED_RENDDESC_V2,
                                    ROUTER_PURPOSE_BRIDGE, NULL));
 done: ;
}

static void
test_dir_purpose_needs_anonymity_returns_false_for_own_bridge_desc(void *arg)
{
  (void)arg;
  tt_int_op(0, OP_EQ, purpose_needs_anonymity(DIR_PURPOSE_FETCH_SERVERDESC,
                                           ROUTER_PURPOSE_BRIDGE,
                                           "authority.z"));
 done: ;
}

static void
test_dir_purpose_needs_anonymity_returns_true_for_sensitive_purpose(void *arg)
{
  (void)arg;

  tt_int_op(1, OP_EQ, purpose_needs_anonymity(
                    DIR_PURPOSE_HAS_FETCHED_RENDDESC_V2,
                    ROUTER_PURPOSE_GENERAL, NULL));
  tt_int_op(1, OP_EQ, purpose_needs_anonymity(
                    DIR_PURPOSE_UPLOAD_RENDDESC_V2, 0,  NULL));
  tt_int_op(1, OP_EQ, purpose_needs_anonymity(
                    DIR_PURPOSE_FETCH_RENDDESC_V2, 0, NULL));
 done: ;
}

static void
test_dir_purpose_needs_anonymity_ret_false_for_non_sensitive_conn(void *arg)
{
  (void)arg;

  tt_int_op(0, OP_EQ, purpose_needs_anonymity(DIR_PURPOSE_UPLOAD_DIR,
                                           ROUTER_PURPOSE_GENERAL, NULL));
  tt_int_op(0, OP_EQ,
            purpose_needs_anonymity(DIR_PURPOSE_UPLOAD_VOTE, 0, NULL));
  tt_int_op(0, OP_EQ,
            purpose_needs_anonymity(DIR_PURPOSE_UPLOAD_SIGNATURES, 0, NULL));
  tt_int_op(0, OP_EQ,
            purpose_needs_anonymity(DIR_PURPOSE_FETCH_STATUS_VOTE, 0, NULL));
  tt_int_op(0, OP_EQ, purpose_needs_anonymity(
                    DIR_PURPOSE_FETCH_DETACHED_SIGNATURES, 0, NULL));
  tt_int_op(0, OP_EQ,
            purpose_needs_anonymity(DIR_PURPOSE_FETCH_CONSENSUS, 0, NULL));
  tt_int_op(0, OP_EQ,
            purpose_needs_anonymity(DIR_PURPOSE_FETCH_CERTIFICATE, 0, NULL));
  tt_int_op(0, OP_EQ,
            purpose_needs_anonymity(DIR_PURPOSE_FETCH_SERVERDESC, 0, NULL));
  tt_int_op(0, OP_EQ,
            purpose_needs_anonymity(DIR_PURPOSE_FETCH_EXTRAINFO, 0, NULL));
  tt_int_op(0, OP_EQ,
            purpose_needs_anonymity(DIR_PURPOSE_FETCH_MICRODESC, 0, NULL));
  done: ;
}

static void
test_dir_fetch_type(void *arg)
{
  (void)arg;

  tt_int_op(dir_fetch_type(DIR_PURPOSE_FETCH_EXTRAINFO, ROUTER_PURPOSE_BRIDGE,
                           NULL), OP_EQ, EXTRAINFO_DIRINFO | BRIDGE_DIRINFO);
  tt_int_op(dir_fetch_type(DIR_PURPOSE_FETCH_EXTRAINFO, ROUTER_PURPOSE_GENERAL,
                           NULL), OP_EQ, EXTRAINFO_DIRINFO | V3_DIRINFO);

  tt_int_op(dir_fetch_type(DIR_PURPOSE_FETCH_SERVERDESC, ROUTER_PURPOSE_BRIDGE,
                           NULL), OP_EQ, BRIDGE_DIRINFO);
  tt_int_op(dir_fetch_type(DIR_PURPOSE_FETCH_SERVERDESC,
                           ROUTER_PURPOSE_GENERAL, NULL), OP_EQ, V3_DIRINFO);

  tt_int_op(dir_fetch_type(DIR_PURPOSE_FETCH_STATUS_VOTE,
                           ROUTER_PURPOSE_GENERAL, NULL), OP_EQ, V3_DIRINFO);
  tt_int_op(dir_fetch_type(DIR_PURPOSE_FETCH_DETACHED_SIGNATURES,
                           ROUTER_PURPOSE_GENERAL, NULL), OP_EQ, V3_DIRINFO);
  tt_int_op(dir_fetch_type(DIR_PURPOSE_FETCH_CERTIFICATE,
                           ROUTER_PURPOSE_GENERAL, NULL), OP_EQ, V3_DIRINFO);

  tt_int_op(dir_fetch_type(DIR_PURPOSE_FETCH_CONSENSUS, ROUTER_PURPOSE_GENERAL,
                           "microdesc"), OP_EQ, V3_DIRINFO|MICRODESC_DIRINFO);
  tt_int_op(dir_fetch_type(DIR_PURPOSE_FETCH_CONSENSUS, ROUTER_PURPOSE_GENERAL,
                           NULL), OP_EQ, V3_DIRINFO);

  tt_int_op(dir_fetch_type(DIR_PURPOSE_FETCH_MICRODESC, ROUTER_PURPOSE_GENERAL,
                           NULL), OP_EQ, MICRODESC_DIRINFO);

  /* This will give a warning, because this function isn't supposed to be
   * used for HS descriptors. */
  setup_full_capture_of_logs(LOG_WARN);
  tt_int_op(dir_fetch_type(DIR_PURPOSE_FETCH_RENDDESC_V2,
                           ROUTER_PURPOSE_GENERAL, NULL), OP_EQ, NO_DIRINFO);
  expect_single_log_msg_containing("Unexpected purpose");
 done:
  teardown_capture_of_logs();
}

static void
test_dir_packages(void *arg)
{
  smartlist_t *votes = smartlist_new();
  char *res = NULL;
  (void)arg;

#define BAD(s) \
  tt_int_op(0, OP_EQ, validate_recommended_package_line(s));
#define GOOD(s) \
  tt_int_op(1, OP_EQ, validate_recommended_package_line(s));
  GOOD("tor 0.2.6.3-alpha "
       "http://torproject.example.com/dist/tor-0.2.6.3-alpha.tar.gz "
       "sha256=sssdlkfjdsklfjdskfljasdklfj");
  GOOD("tor 0.2.6.3-alpha "
       "http://torproject.example.com/dist/tor-0.2.6.3-alpha.tar.gz "
       "sha256=sssdlkfjdsklfjdskfljasdklfj blake2b=fred");
  BAD("tor 0.2.6.3-alpha "
       "http://torproject.example.com/dist/tor-0.2.6.3-alpha.tar.gz "
       "sha256=sssdlkfjdsklfjdskfljasdklfj=");
  BAD("tor 0.2.6.3-alpha "
       "http://torproject.example.com/dist/tor-0.2.6.3-alpha.tar.gz "
       "sha256=sssdlkfjdsklfjdskfljasdklfj blake2b");
  BAD("tor 0.2.6.3-alpha "
       "http://torproject.example.com/dist/tor-0.2.6.3-alpha.tar.gz ");
  BAD("tor 0.2.6.3-alpha "
       "http://torproject.example.com/dist/tor-0.2.6.3-alpha.tar.gz");
  BAD("tor 0.2.6.3-alpha ");
  BAD("tor 0.2.6.3-alpha");
  BAD("tor ");
  BAD("tor");
  BAD("");
  BAD("=foobar sha256="
      "3c179f46ca77069a6a0bac70212a9b3b838b2f66129cb52d568837fc79d8fcc7");
  BAD("= = sha256="
      "3c179f46ca77069a6a0bac70212a9b3b838b2f66129cb52d568837fc79d8fcc7");

  BAD("sha512= sha256="
      "3c179f46ca77069a6a0bac70212a9b3b838b2f66129cb52d568837fc79d8fcc7");

  smartlist_add(votes, tor_malloc_zero(sizeof(networkstatus_t)));
  smartlist_add(votes, tor_malloc_zero(sizeof(networkstatus_t)));
  smartlist_add(votes, tor_malloc_zero(sizeof(networkstatus_t)));
  smartlist_add(votes, tor_malloc_zero(sizeof(networkstatus_t)));
  smartlist_add(votes, tor_malloc_zero(sizeof(networkstatus_t)));
  smartlist_add(votes, tor_malloc_zero(sizeof(networkstatus_t)));
  SMARTLIST_FOREACH(votes, networkstatus_t *, ns,
                    ns->package_lines = smartlist_new());

#define ADD(i, s)                                                       \
  smartlist_add(((networkstatus_t*)smartlist_get(votes, (i)))->package_lines, \
                (void*)(s));

  /* Only one vote for this one. */
  ADD(4, "cisco 99z http://foobar.example.com/ sha256=blahblah");

  /* Only two matching entries for this one, but 3 voters */
  ADD(1, "mystic 99y http://barfoo.example.com/ sha256=blahblah");
  ADD(3, "mystic 99y http://foobar.example.com/ sha256=blahblah");
  ADD(4, "mystic 99y http://foobar.example.com/ sha256=blahblah");

  /* Only two matching entries for this one, but at least 4 voters */
  ADD(1, "mystic 99p http://barfoo.example.com/ sha256=ggggggg");
  ADD(3, "mystic 99p http://foobar.example.com/ sha256=blahblah");
  ADD(4, "mystic 99p http://foobar.example.com/ sha256=blahblah");
  ADD(5, "mystic 99p http://foobar.example.com/ sha256=ggggggg");

  /* This one has only invalid votes. */
  ADD(0, "haffenreffer 1.2 http://foobar.example.com/ sha256");
  ADD(1, "haffenreffer 1.2 http://foobar.example.com/ ");
  ADD(2, "haffenreffer 1.2 ");
  ADD(3, "haffenreffer ");
  ADD(4, "haffenreffer");

  /* Three matching votes for this; it should actually go in! */
  ADD(2, "element 0.66.1 http://quux.example.com/ sha256=abcdef");
  ADD(3, "element 0.66.1 http://quux.example.com/ sha256=abcdef");
  ADD(4, "element 0.66.1 http://quux.example.com/ sha256=abcdef");
  ADD(1, "element 0.66.1 http://quum.example.com/ sha256=abcdef");
  ADD(0, "element 0.66.1 http://quux.example.com/ sha256=abcde");

  /* Three votes for A, three votes for B */
  ADD(0, "clownshoes 22alpha1 http://quumble.example.com/ blake2=foob");
  ADD(1, "clownshoes 22alpha1 http://quumble.example.com/ blake2=foob");
  ADD(2, "clownshoes 22alpha1 http://quumble.example.com/ blake2=foob");
  ADD(3, "clownshoes 22alpha1 http://quumble.example.com/ blake2=fooz");
  ADD(4, "clownshoes 22alpha1 http://quumble.example.com/ blake2=fooz");
  ADD(5, "clownshoes 22alpha1 http://quumble.example.com/ blake2=fooz");

  /* Three votes for A, two votes for B */
  ADD(1, "clownshoes 22alpha3 http://quumble.example.com/ blake2=foob");
  ADD(2, "clownshoes 22alpha3 http://quumble.example.com/ blake2=foob");
  ADD(3, "clownshoes 22alpha3 http://quumble.example.com/ blake2=fooz");
  ADD(4, "clownshoes 22alpha3 http://quumble.example.com/ blake2=fooz");
  ADD(5, "clownshoes 22alpha3 http://quumble.example.com/ blake2=fooz");

  /* Four votes for A, two for B. */
  ADD(0, "clownshoes 22alpha4 http://quumble.example.com/ blake2=foob");
  ADD(1, "clownshoes 22alpha4 http://quumble.example.com/ blake2=foob");
  ADD(2, "clownshoes 22alpha4 http://quumble.example.cam/ blake2=fooa");
  ADD(3, "clownshoes 22alpha4 http://quumble.example.cam/ blake2=fooa");
  ADD(4, "clownshoes 22alpha4 http://quumble.example.cam/ blake2=fooa");
  ADD(5, "clownshoes 22alpha4 http://quumble.example.cam/ blake2=fooa");

  /* Five votes for A ... all from the same authority.  Three for B. */
  ADD(0, "cbc 99.1.11.1.1 http://example.com/cbc/ cubehash=ahooy sha512=m");
  ADD(1, "cbc 99.1.11.1.1 http://example.com/cbc/ cubehash=ahooy sha512=m");
  ADD(3, "cbc 99.1.11.1.1 http://example.com/cbc/ cubehash=ahooy sha512=m");
  ADD(2, "cbc 99.1.11.1.1 http://example.com/ cubehash=ahooy");
  ADD(2, "cbc 99.1.11.1.1 http://example.com/ cubehash=ahooy");
  ADD(2, "cbc 99.1.11.1.1 http://example.com/ cubehash=ahooy");
  ADD(2, "cbc 99.1.11.1.1 http://example.com/ cubehash=ahooy");
  ADD(2, "cbc 99.1.11.1.1 http://example.com/ cubehash=ahooy");

  /* As above but new replaces old: no two match. */
  ADD(0, "cbc 99.1.11.1.2 http://example.com/cbc/ cubehash=ahooy sha512=m");
  ADD(1, "cbc 99.1.11.1.2 http://example.com/cbc/ cubehash=ahooy sha512=m");
  ADD(1, "cbc 99.1.11.1.2 http://example.com/cbc/x cubehash=ahooy sha512=m");
  ADD(2, "cbc 99.1.11.1.2 http://example.com/cbc/ cubehash=ahooy sha512=m");
  ADD(2, "cbc 99.1.11.1.2 http://example.com/ cubehash=ahooy");
  ADD(2, "cbc 99.1.11.1.2 http://example.com/ cubehash=ahooy");
  ADD(2, "cbc 99.1.11.1.2 http://example.com/ cubehash=ahooy");
  ADD(2, "cbc 99.1.11.1.2 http://example.com/ cubehash=ahooy");
  ADD(2, "cbc 99.1.11.1.2 http://example.com/ cubehash=ahooy");

  res = compute_consensus_package_lines(votes);
  tt_assert(res);
  tt_str_op(res, OP_EQ,
    "package cbc 99.1.11.1.1 http://example.com/cbc/ cubehash=ahooy sha512=m\n"
    "package clownshoes 22alpha3 http://quumble.example.com/ blake2=fooz\n"
    "package clownshoes 22alpha4 http://quumble.example.cam/ blake2=fooa\n"
    "package element 0.66.1 http://quux.example.com/ sha256=abcdef\n"
    "package mystic 99y http://foobar.example.com/ sha256=blahblah\n"
            );

#undef ADD
#undef BAD
#undef GOOD
 done:
  SMARTLIST_FOREACH(votes, networkstatus_t *, ns,
                    { smartlist_free(ns->package_lines); tor_free(ns); });
  smartlist_free(votes);
  tor_free(res);
}

static void
download_status_random_backoff_helper(int min_delay)
{
  download_status_t dls_random =
    { 0, 0, 0, DL_SCHED_GENERIC, DL_WANT_AUTHORITY,
               DL_SCHED_INCREMENT_FAILURE, 0, 0 };
  int increment = -1;
  int old_increment = -1;
  time_t current_time = time(NULL);

  /* Check the random backoff cases */
  int n_attempts = 0;
  do {
    increment = download_status_schedule_get_delay(&dls_random,
                                                   min_delay,
                                                   current_time);

    log_debug(LD_DIR, "Min: %d, Inc: %d, Old Inc: %d",
              min_delay, increment, old_increment);

    /* Regression test for 20534 and friends
     * increment must always increase after the first */
    if (dls_random.last_backoff_position > 0) {
      /* Always increment the exponential backoff */
      tt_int_op(increment, OP_GE, 1);
    }

    /* Test */
    tt_int_op(increment, OP_GE, min_delay);

    /* Advance */
    if (dls_random.n_download_attempts < IMPOSSIBLE_TO_DOWNLOAD - 1) {
      ++(dls_random.n_download_attempts);
      ++(dls_random.n_download_failures);
    }

    /* Try another maybe */
    old_increment = increment;
  } while (++n_attempts < 1000);

 done:
  return;
}

static void
test_dir_download_status_random_backoff(void *arg)
{
  (void)arg;

  /* Do a standard test */
  download_status_random_backoff_helper(0);
  /* regression tests for 17750: initial delay */
  download_status_random_backoff_helper(10);
  download_status_random_backoff_helper(20);

  /* Pathological cases */
  download_status_random_backoff_helper(INT_MAX/2);
}

static void
test_dir_download_status_random_backoff_ranges(void *arg)
{
  (void)arg;
  int lo, hi;
  next_random_exponential_delay_range(&lo, &hi, 0, 10);
  tt_int_op(lo, OP_EQ, 10);
  tt_int_op(hi, OP_EQ, 11);

  next_random_exponential_delay_range(&lo, &hi, 6, 10);
  tt_int_op(lo, OP_EQ, 10);
  tt_int_op(hi, OP_EQ, 6*3);

  next_random_exponential_delay_range(&lo, &hi, 13, 10);
  tt_int_op(lo, OP_EQ, 10);
  tt_int_op(hi, OP_EQ, 13 * 3);

  next_random_exponential_delay_range(&lo, &hi, 37, 10);
  tt_int_op(lo, OP_EQ, 10);
  tt_int_op(hi, OP_EQ, 111);

  next_random_exponential_delay_range(&lo, &hi, 123, 10);
  tt_int_op(lo, OP_EQ, 10);
  tt_int_op(hi, OP_EQ, 369);

  next_random_exponential_delay_range(&lo, &hi, INT_MAX-5, 10);
  tt_int_op(lo, OP_EQ, 10);
  tt_int_op(hi, OP_EQ, INT_MAX);
 done:
  ;
}

static void
test_dir_download_status_increment(void *arg)
{
  (void)arg;
  download_status_t dls_exp = { 0, 0, 0, DL_SCHED_GENERIC,
    DL_WANT_ANY_DIRSERVER,
    DL_SCHED_INCREMENT_ATTEMPT,
    0, 0 };
  or_options_t test_options;
  time_t current_time = time(NULL);

  const int delay0 = 10;
  const int no_delay = 0;
  const int schedule = 10;
  const int schedule_no_initial_delay = 0;

  /* Put it in the options */
  mock_options = &test_options;
  reset_options(mock_options, &mock_get_options_calls);
  mock_options->TestingBridgeBootstrapDownloadInitialDelay = schedule;
  mock_options->TestingClientDownloadInitialDelay = schedule;

  MOCK(get_options, mock_get_options);

  /* Check that the initial value of the schedule is the first value used,
   * whether or not it was reset before being used */

  /* regression test for 17750: no initial delay */
  mock_options->TestingClientDownloadInitialDelay = schedule_no_initial_delay;
  mock_get_options_calls = 0;
  /* we really want to test that it's equal to time(NULL) + delay0, but that's
   * an unrealiable test, because time(NULL) might change. */

  /* regression test for 17750: exponential, no initial delay */
  mock_options->TestingClientDownloadInitialDelay = schedule_no_initial_delay;
  mock_get_options_calls = 0;
  /* we really want to test that it's equal to time(NULL) + delay0, but that's
   * an unrealiable test, because time(NULL) might change. */
  tt_assert(download_status_get_next_attempt_at(&dls_exp)
            >= current_time + no_delay);
  tt_assert(download_status_get_next_attempt_at(&dls_exp)
            != TIME_MAX);
  tt_int_op(download_status_get_n_failures(&dls_exp), OP_EQ, 0);
  tt_int_op(download_status_get_n_attempts(&dls_exp), OP_EQ, 0);
  tt_int_op(mock_get_options_calls, OP_GE, 1);

  /* regression test for 17750: exponential, initial delay */
  mock_options->TestingClientDownloadInitialDelay = schedule;
  mock_get_options_calls = 0;
  /* we really want to test that it's equal to time(NULL) + delay0, but that's
   * an unrealiable test, because time(NULL) might change. */
  tt_assert(download_status_get_next_attempt_at(&dls_exp)
            >= current_time + delay0);
  tt_assert(download_status_get_next_attempt_at(&dls_exp)
            != TIME_MAX);
  tt_int_op(download_status_get_n_failures(&dls_exp), OP_EQ, 0);
  tt_int_op(download_status_get_n_attempts(&dls_exp), OP_EQ, 0);
  tt_int_op(mock_get_options_calls, OP_GE, 1);

 done:
  UNMOCK(get_options);
  mock_options = NULL;
  mock_get_options_calls = 0;
  teardown_capture_of_logs();
}

static void
test_dir_authdir_type_to_string(void *data)
{
  (void)data;
  char *res;

  tt_str_op(res = authdir_type_to_string(NO_DIRINFO), OP_EQ,
            "[Not an authority]");
  tor_free(res);

  tt_str_op(res = authdir_type_to_string(EXTRAINFO_DIRINFO), OP_EQ,
            "[Not an authority]");
  tor_free(res);

  tt_str_op(res = authdir_type_to_string(MICRODESC_DIRINFO), OP_EQ,
            "[Not an authority]");
  tor_free(res);

  tt_str_op(res = authdir_type_to_string(V3_DIRINFO), OP_EQ, "V3");
  tor_free(res);

  tt_str_op(res = authdir_type_to_string(BRIDGE_DIRINFO), OP_EQ, "Bridge");
  tor_free(res);

  tt_str_op(res = authdir_type_to_string(
            V3_DIRINFO | BRIDGE_DIRINFO | EXTRAINFO_DIRINFO), OP_EQ,
            "V3, Bridge");
  done:
  tor_free(res);
}

static void
test_dir_conn_purpose_to_string(void *data)
{
  (void)data;

#define EXPECT_CONN_PURPOSE(purpose, expected) \
  tt_str_op(dir_conn_purpose_to_string(purpose), OP_EQ, expected);

  EXPECT_CONN_PURPOSE(DIR_PURPOSE_UPLOAD_DIR, "server descriptor upload");
  EXPECT_CONN_PURPOSE(DIR_PURPOSE_UPLOAD_VOTE, "server vote upload");
  EXPECT_CONN_PURPOSE(DIR_PURPOSE_UPLOAD_SIGNATURES,
                      "consensus signature upload");
  EXPECT_CONN_PURPOSE(DIR_PURPOSE_FETCH_SERVERDESC, "server descriptor fetch");
  EXPECT_CONN_PURPOSE(DIR_PURPOSE_FETCH_EXTRAINFO, "extra-info fetch");
  EXPECT_CONN_PURPOSE(DIR_PURPOSE_FETCH_CONSENSUS,
                      "consensus network-status fetch");
  EXPECT_CONN_PURPOSE(DIR_PURPOSE_FETCH_CERTIFICATE, "authority cert fetch");
  EXPECT_CONN_PURPOSE(DIR_PURPOSE_FETCH_STATUS_VOTE, "status vote fetch");
  EXPECT_CONN_PURPOSE(DIR_PURPOSE_FETCH_DETACHED_SIGNATURES,
                      "consensus signature fetch");
  EXPECT_CONN_PURPOSE(DIR_PURPOSE_FETCH_RENDDESC_V2,
                      "hidden-service v2 descriptor fetch");
  EXPECT_CONN_PURPOSE(DIR_PURPOSE_UPLOAD_RENDDESC_V2,
                      "hidden-service v2 descriptor upload");
  EXPECT_CONN_PURPOSE(DIR_PURPOSE_FETCH_MICRODESC, "microdescriptor fetch");

  /* This will give a warning, because there is no purpose 1024. */
  setup_full_capture_of_logs(LOG_WARN);
  EXPECT_CONN_PURPOSE(1024, "(unknown)");
  expect_single_log_msg_containing("Called with unknown purpose 1024");

 done:
  teardown_capture_of_logs();
}

NS_DECL(int,
public_server_mode, (const or_options_t *options));

static int
NS(public_server_mode)(const or_options_t *options)
{
  (void)options;

  if (CALLED(public_server_mode)++ == 0) {
    return 1;
  }

  return 0;
}

static void
test_dir_should_use_directory_guards(void *data)
{
  or_options_t *options;
  char *errmsg = NULL;
  (void)data;

  NS_MOCK(public_server_mode);

  options = options_new();
  options_init(options);

  tt_int_op(should_use_directory_guards(options), OP_EQ, 0);
  tt_int_op(CALLED(public_server_mode), OP_EQ, 1);

  options->UseEntryGuards = 1;
  options->DownloadExtraInfo = 0;
  options->FetchDirInfoEarly = 0;
  options->FetchDirInfoExtraEarly = 0;
  options->FetchUselessDescriptors = 0;
  tt_int_op(should_use_directory_guards(options), OP_EQ, 1);
  tt_int_op(CALLED(public_server_mode), OP_EQ, 2);

  options->UseEntryGuards = 0;
  tt_int_op(should_use_directory_guards(options), OP_EQ, 0);
  tt_int_op(CALLED(public_server_mode), OP_EQ, 3);
  options->UseEntryGuards = 1;

  options->DownloadExtraInfo = 1;
  tt_int_op(should_use_directory_guards(options), OP_EQ, 0);
  tt_int_op(CALLED(public_server_mode), OP_EQ, 4);
  options->DownloadExtraInfo = 0;

  options->FetchDirInfoEarly = 1;
  tt_int_op(should_use_directory_guards(options), OP_EQ, 0);
  tt_int_op(CALLED(public_server_mode), OP_EQ, 5);
  options->FetchDirInfoEarly = 0;

  options->FetchDirInfoExtraEarly = 1;
  tt_int_op(should_use_directory_guards(options), OP_EQ, 0);
  tt_int_op(CALLED(public_server_mode), OP_EQ, 6);
  options->FetchDirInfoExtraEarly = 0;

  options->FetchUselessDescriptors = 1;
  tt_int_op(should_use_directory_guards(options), OP_EQ, 0);
  tt_int_op(CALLED(public_server_mode), OP_EQ, 7);
  options->FetchUselessDescriptors = 0;

  done:
    NS_UNMOCK(public_server_mode);
    or_options_free(options);
    tor_free(errmsg);
}

NS_DECL(void,
directory_initiate_request, (directory_request_t *req));

static void
test_dir_should_not_init_request_to_ourselves(void *data)
{
  char digest[DIGEST_LEN];
  dir_server_t *ourself = NULL;
  crypto_pk_t *key = pk_generate(2);
  (void) data;

  NS_MOCK(directory_initiate_request);

  clear_dir_servers();
  routerlist_free_all();

  set_server_identity_key(key);
  crypto_pk_get_digest(key, (char*) &digest);
  ourself = trusted_dir_server_new("ourself", "127.0.0.1", 9059, 9060,
                                   NULL, digest,
                                   NULL, V3_DIRINFO, 1.0);

  tt_assert(ourself);
  dir_server_add(ourself);

  directory_get_from_all_authorities(DIR_PURPOSE_FETCH_STATUS_VOTE, 0, NULL);
  tt_int_op(CALLED(directory_initiate_request), OP_EQ, 0);

  directory_get_from_all_authorities(DIR_PURPOSE_FETCH_DETACHED_SIGNATURES, 0,
                                     NULL);

  tt_int_op(CALLED(directory_initiate_request), OP_EQ, 0);

  done:
    NS_UNMOCK(directory_initiate_request);
    clear_dir_servers();
    routerlist_free_all();
    crypto_pk_free(key);
}

static void
test_dir_should_not_init_request_to_dir_auths_without_v3_info(void *data)
{
  dir_server_t *ds = NULL;
  dirinfo_type_t dirinfo_type = BRIDGE_DIRINFO | EXTRAINFO_DIRINFO \
                                | MICRODESC_DIRINFO;
  (void) data;

  NS_MOCK(directory_initiate_request);

  clear_dir_servers();
  routerlist_free_all();

  ds = trusted_dir_server_new("ds", "10.0.0.1", 9059, 9060, NULL,
                              "12345678901234567890", NULL, dirinfo_type, 1.0);
  tt_assert(ds);
  dir_server_add(ds);

  directory_get_from_all_authorities(DIR_PURPOSE_FETCH_STATUS_VOTE, 0, NULL);
  tt_int_op(CALLED(directory_initiate_request), OP_EQ, 0);

  directory_get_from_all_authorities(DIR_PURPOSE_FETCH_DETACHED_SIGNATURES, 0,
                                     NULL);
  tt_int_op(CALLED(directory_initiate_request), OP_EQ, 0);

  done:
    NS_UNMOCK(directory_initiate_request);
    clear_dir_servers();
    routerlist_free_all();
}

static void
test_dir_should_init_request_to_dir_auths(void *data)
{
  dir_server_t *ds = NULL;
  (void) data;

  NS_MOCK(directory_initiate_request);

  clear_dir_servers();
  routerlist_free_all();

  ds = trusted_dir_server_new("ds", "10.0.0.1", 9059, 9060, NULL,
                              "12345678901234567890", NULL, V3_DIRINFO, 1.0);
  tt_assert(ds);
  dir_server_add(ds);

  directory_get_from_all_authorities(DIR_PURPOSE_FETCH_STATUS_VOTE, 0, NULL);
  tt_int_op(CALLED(directory_initiate_request), OP_EQ, 1);

  directory_get_from_all_authorities(DIR_PURPOSE_FETCH_DETACHED_SIGNATURES, 0,
                                     NULL);
  tt_int_op(CALLED(directory_initiate_request), OP_EQ, 2);

  done:
    NS_UNMOCK(directory_initiate_request);
    clear_dir_servers();
    routerlist_free_all();
}

void
NS(directory_initiate_request)(directory_request_t *req)
{
  (void)req;
  CALLED(directory_initiate_request)++;
}

static void
test_dir_choose_compression_level(void* data)
{
  (void)data;

  /* It starts under_memory_pressure */
  tt_int_op(have_been_under_memory_pressure(), OP_EQ, 1);

  tt_assert(HIGH_COMPRESSION == choose_compression_level(-1));
  tt_assert(LOW_COMPRESSION == choose_compression_level(1024-1));
  tt_assert(MEDIUM_COMPRESSION == choose_compression_level(2048-1));
  tt_assert(HIGH_COMPRESSION == choose_compression_level(2048));

  /* Reset under_memory_pressure timer */
  cell_queues_check_size();
  tt_int_op(have_been_under_memory_pressure(), OP_EQ, 0);

  tt_assert(HIGH_COMPRESSION == choose_compression_level(-1));
  tt_assert(HIGH_COMPRESSION == choose_compression_level(1024-1));
  tt_assert(HIGH_COMPRESSION == choose_compression_level(2048-1));
  tt_assert(HIGH_COMPRESSION == choose_compression_level(2048));

  done: ;
}

/*
 * Mock check_private_dir(), and always succeed - no need to actually
 * look at or create anything on the filesystem.
 */

static int
mock_check_private_dir(const char *dirname, cpd_check_t check,
                       const char *effective_user)
{
  (void)dirname;
  (void)check;
  (void)effective_user;

  return 0;
}

/*
 * This really mocks options_get_datadir_fname2_suffix(), but for testing
 * dump_desc(), we only care about get_datadir_fname(sub1), which is defined
 * in config.h as:
 *
 * options_get_datadir_fname2_suffix(get_options(), sub1, NULL, NULL)
 */

static char *
mock_get_datadir_fname(const or_options_t *options,
                       directory_root_t roottype,
                       const char *sub1, const char *sub2,
                       const char *suffix)
{
  (void) roottype;
  char *rv = NULL;

  /*
   * Assert we were called like get_datadir_fname2() or get_datadir_fname(),
   * since that's all we implement here.
   */
  tt_ptr_op(options, OP_NE, NULL);
  tt_ptr_op(sub1, OP_NE, NULL);
  /*
   * No particular assertions about sub2, since we could be in the
   * get_datadir_fname() or get_datadir_fname2() case.
   */
  tt_ptr_op(suffix, OP_EQ, NULL);

  /* Just duplicate the basename and return it for this mock */
  if (sub2) {
    /* If we have sub2, it's the basename, otherwise sub1 */
    rv = tor_strdup(sub2);
  } else {
    rv = tor_strdup(sub1);
  }

 done:
  return rv;
}

static char *last_unlinked_path = NULL;
static int unlinked_count = 0;

static void
mock_unlink_reset(void)
{
  tor_free(last_unlinked_path);
  unlinked_count = 0;
}

static int
mock_unlink(const char *path)
{
  tt_ptr_op(path, OP_NE, NULL);

  tor_free(last_unlinked_path);
  last_unlinked_path = tor_strdup(path);
  ++unlinked_count;

 done:
  return 0;
}

static char *last_write_str_path = NULL;
static uint8_t last_write_str_hash[DIGEST256_LEN];
static int write_str_count = 0;

static void
mock_write_str_to_file_reset(void)
{
  tor_free(last_write_str_path);
  write_str_count = 0;
}

static int
mock_write_str_to_file(const char *path, const char *str, int bin)
{
  size_t len;
  uint8_t hash[DIGEST256_LEN];

  (void)bin;

  tt_ptr_op(path, OP_NE, NULL);
  tt_ptr_op(str, OP_NE, NULL);

  len = strlen(str);
  crypto_digest256((char *)hash, str, len, DIGEST_SHA256);

  tor_free(last_write_str_path);
  last_write_str_path = tor_strdup(path);
  memcpy(last_write_str_hash, hash, sizeof(last_write_str_hash));
  ++write_str_count;

 done:
  return 0;
}

static void
test_dir_dump_unparseable_descriptors(void *data)
{
  /*
   * These bogus descriptors look nothing at all like real bogus descriptors
   * we might see, but we're only testing dump_desc() here, not the parser.
   */
  const char *test_desc_type = "squamous";
  /* strlen(test_desc_1) = 583 bytes */
  const char *test_desc_1 =
    "The most merciful thing in the world, I think, is the inability of the "
    "human mind to correlate all its contents. We live on a placid island of"
    " ignorance in the midst of black seas of infinity, and it was not meant"
    " that we should voyage far. The sciences, each straining in its own dir"
    "ection, have hitherto harmed us little; but some day the piecing togeth"
    "er of dissociated knowledge will open up such terrifying vistas of real"
    "ity, and of our frightful position therein, that we shall either go mad"
    "from the revelation or flee from the light into the peace and safety of"
    "a new dark age.";
  uint8_t test_desc_1_hash[DIGEST256_LEN];
  char test_desc_1_hash_str[HEX_DIGEST256_LEN+1];
  /* strlen(test_desc_2) = 650 bytes */
  const char *test_desc_2 =
    "I think their predominant colour was a greyish-green, though they had w"
    "hite bellies. They were mostly shiny and slippery, but the ridges of th"
    "eir backs were scaly. Their forms vaguely suggested the anthropoid, whi"
    "le their heads were the heads of fish, with prodigious bulging eyes tha"
    "t never closed. At the sides of their necks were palpitating gills, and"
    "their long paws were webbed. They hopped irregularly, sometimes on two "
    "legs and sometimes on four. I was somehow glad that they had no more th"
    "an four limbs. Their croaking, baying voices, clearly wed tar articulat"
    "e speech, held all the dark shades of expression which their staring fa"
    "ces lacked.";
  uint8_t test_desc_2_hash[DIGEST256_LEN];
  char test_desc_2_hash_str[HEX_DIGEST256_LEN+1];
  /* strlen(test_desc_3) = 700 bytes */
  const char *test_desc_3 =
    "Without knowing what futurism is like, Johansen achieved something very"
    "close to it when he spoke of the city; for instead of describing any de"
    "finite structure or building, he dwells only on broad impressions of va"
    "st angles and stone surfaces - surfaces too great to belong to anything"
    "right or proper for this earth, and impious with horrible images and hi"
    "eroglyphs. I mention his talk about angles because it suggests somethin"
    "g Wilcox had told me of his awful dreams. He said that the geometry of "
    "the dream-place he saw was abnormal, non-Euclidean, and loathsomely red"
    "olent of spheres and dimensions apart from ours. Now an unlettered seam"
    "an felt the same thing whilst gazing at the terrible reality.";
  uint8_t test_desc_3_hash[DIGEST256_LEN];
  char test_desc_3_hash_str[HEX_DIGEST256_LEN+1];
  /* strlen(test_desc_3) = 604 bytes */
  const char *test_desc_4 =
    "So we glanced back simultaneously, it would appear; though no doubt the"
    "incipient motion of one prompted the imitation of the other. As we did "
    "so we flashed both torches full strength at the momentarily thinned mis"
    "t; either from sheer primitive anxiety to see all we could, or in a les"
    "s primitive but equally unconscious effort to dazzle the entity before "
    "we dimmed our light and dodged among the penguins of the labyrinth cent"
    "er ahead. Unhappy act! Not Orpheus himself, or Lot's wife, paid much mo"
    "re dearly for a backward glance. And again came that shocking, wide-ran"
    "ged piping - \"Tekeli-li! Tekeli-li!\"";
  uint8_t test_desc_4_hash[DIGEST256_LEN];
  char test_desc_4_hash_str[HEX_DIGEST256_LEN+1];
  (void)data;

  /*
   * Set up options mock so we can force a tiny FIFO size and generate
   * cleanups.
   */
  mock_options = tor_malloc(sizeof(or_options_t));
  reset_options(mock_options, &mock_get_options_calls);
  mock_options->MaxUnparseableDescSizeToLog = 1536;
  MOCK(get_options, mock_get_options);
  MOCK(check_private_dir, mock_check_private_dir);
  MOCK(options_get_dir_fname2_suffix,
       mock_get_datadir_fname);

  /*
   * Set up unlink and write mocks
   */
  MOCK(tor_unlink, mock_unlink);
  mock_unlink_reset();
  MOCK(write_str_to_file, mock_write_str_to_file);
  mock_write_str_to_file_reset();

  /*
   * Compute hashes we'll need to recognize which descriptor is which
   */
  crypto_digest256((char *)test_desc_1_hash, test_desc_1,
                   strlen(test_desc_1), DIGEST_SHA256);
  base16_encode(test_desc_1_hash_str, sizeof(test_desc_1_hash_str),
                (const char *)test_desc_1_hash,
                sizeof(test_desc_1_hash));
  crypto_digest256((char *)test_desc_2_hash, test_desc_2,
                   strlen(test_desc_2), DIGEST_SHA256);
  base16_encode(test_desc_2_hash_str, sizeof(test_desc_2_hash_str),
                (const char *)test_desc_2_hash,
                sizeof(test_desc_2_hash));
  crypto_digest256((char *)test_desc_3_hash, test_desc_3,
                   strlen(test_desc_3), DIGEST_SHA256);
  base16_encode(test_desc_3_hash_str, sizeof(test_desc_3_hash_str),
                (const char *)test_desc_3_hash,
                sizeof(test_desc_3_hash));
  crypto_digest256((char *)test_desc_4_hash, test_desc_4,
                   strlen(test_desc_4), DIGEST_SHA256);
  base16_encode(test_desc_4_hash_str, sizeof(test_desc_4_hash_str),
                (const char *)test_desc_4_hash,
                sizeof(test_desc_4_hash));

  /*
   * Reset the FIFO and check its state
   */
  dump_desc_fifo_cleanup();
  tt_u64_op(len_descs_dumped, OP_EQ, 0);
  tt_assert(descs_dumped == NULL || smartlist_len(descs_dumped) == 0);

  /*
   * (1) Fire off dump_desc() once; these descriptors should all be safely
   * smaller than configured FIFO size.
   */

  dump_desc(test_desc_1, test_desc_type);

  /*
   * Assert things about the FIFO state
   */
  tt_u64_op(len_descs_dumped, OP_EQ, strlen(test_desc_1));
  tt_assert(descs_dumped != NULL && smartlist_len(descs_dumped) == 1);

  /*
   * Assert things about the mocks
   */
  tt_int_op(unlinked_count, OP_EQ, 0);
  tt_int_op(write_str_count, OP_EQ, 1);
  tt_mem_op(last_write_str_hash, OP_EQ, test_desc_1_hash, DIGEST_SHA256);

  /*
   * Reset the FIFO and check its state
   */
  dump_desc_fifo_cleanup();
  tt_u64_op(len_descs_dumped, OP_EQ, 0);
  tt_assert(descs_dumped == NULL || smartlist_len(descs_dumped) == 0);

  /*
   * Reset the mocks and check their state
   */
  mock_unlink_reset();
  mock_write_str_to_file_reset();
  tt_int_op(unlinked_count, OP_EQ, 0);
  tt_int_op(write_str_count, OP_EQ, 0);

  /*
   * (2) Fire off dump_desc() twice; this still should trigger no cleanup.
   */

  /* First time */
  dump_desc(test_desc_2, test_desc_type);

  /*
   * Assert things about the FIFO state
   */
  tt_u64_op(len_descs_dumped, OP_EQ, strlen(test_desc_2));
  tt_assert(descs_dumped != NULL && smartlist_len(descs_dumped) == 1);

  /*
   * Assert things about the mocks
   */
  tt_int_op(unlinked_count, OP_EQ, 0);
  tt_int_op(write_str_count, OP_EQ, 1);
  tt_mem_op(last_write_str_hash, OP_EQ, test_desc_2_hash, DIGEST_SHA256);

  /* Second time */
  dump_desc(test_desc_3, test_desc_type);

  /*
   * Assert things about the FIFO state
   */
  tt_u64_op(len_descs_dumped, OP_EQ,
            strlen(test_desc_2) + strlen(test_desc_3));
  tt_assert(descs_dumped != NULL && smartlist_len(descs_dumped) == 2);

  /*
   * Assert things about the mocks
   */
  tt_int_op(unlinked_count, OP_EQ, 0);
  tt_int_op(write_str_count, OP_EQ, 2);
  tt_mem_op(last_write_str_hash, OP_EQ, test_desc_3_hash, DIGEST_SHA256);

  /*
   * Reset the FIFO and check its state
   */
  dump_desc_fifo_cleanup();
  tt_u64_op(len_descs_dumped, OP_EQ, 0);
  tt_assert(descs_dumped == NULL || smartlist_len(descs_dumped) == 0);

  /*
   * Reset the mocks and check their state
   */
  mock_unlink_reset();
  mock_write_str_to_file_reset();
  tt_int_op(unlinked_count, OP_EQ, 0);
  tt_int_op(write_str_count, OP_EQ, 0);

  /*
   * (3) Three calls to dump_desc cause a FIFO cleanup
   */

  /* First time */
  dump_desc(test_desc_4, test_desc_type);

  /*
   * Assert things about the FIFO state
   */
  tt_u64_op(len_descs_dumped, OP_EQ, strlen(test_desc_4));
  tt_assert(descs_dumped != NULL && smartlist_len(descs_dumped) == 1);

  /*
   * Assert things about the mocks
   */
  tt_int_op(unlinked_count, OP_EQ, 0);
  tt_int_op(write_str_count, OP_EQ, 1);
  tt_mem_op(last_write_str_hash, OP_EQ, test_desc_4_hash, DIGEST_SHA256);

  /* Second time */
  dump_desc(test_desc_1, test_desc_type);

  /*
   * Assert things about the FIFO state
   */
  tt_u64_op(len_descs_dumped, OP_EQ,
            strlen(test_desc_4) + strlen(test_desc_1));
  tt_assert(descs_dumped != NULL && smartlist_len(descs_dumped) == 2);

  /*
   * Assert things about the mocks
   */
  tt_int_op(unlinked_count, OP_EQ, 0);
  tt_int_op(write_str_count, OP_EQ, 2);
  tt_mem_op(last_write_str_hash, OP_EQ, test_desc_1_hash, DIGEST_SHA256);

  /* Third time - we should unlink the dump of test_desc_4 here */
  dump_desc(test_desc_2, test_desc_type);

  /*
   * Assert things about the FIFO state
   */
  tt_u64_op(len_descs_dumped, OP_EQ,
            strlen(test_desc_1) + strlen(test_desc_2));
  tt_assert(descs_dumped != NULL && smartlist_len(descs_dumped) == 2);

  /*
   * Assert things about the mocks
   */
  tt_int_op(unlinked_count, OP_EQ, 1);
  tt_int_op(write_str_count, OP_EQ, 3);
  tt_mem_op(last_write_str_hash, OP_EQ, test_desc_2_hash, DIGEST_SHA256);

  /*
   * Reset the FIFO and check its state
   */
  dump_desc_fifo_cleanup();
  tt_u64_op(len_descs_dumped, OP_EQ, 0);
  tt_assert(descs_dumped == NULL || smartlist_len(descs_dumped) == 0);

  /*
   * Reset the mocks and check their state
   */
  mock_unlink_reset();
  mock_write_str_to_file_reset();
  tt_int_op(unlinked_count, OP_EQ, 0);
  tt_int_op(write_str_count, OP_EQ, 0);

  /*
   * (4) But repeating one (A B B) doesn't overflow and cleanup
   */

  /* First time */
  dump_desc(test_desc_3, test_desc_type);

  /*
   * Assert things about the FIFO state
   */
  tt_u64_op(len_descs_dumped, OP_EQ, strlen(test_desc_3));
  tt_assert(descs_dumped != NULL && smartlist_len(descs_dumped) == 1);

  /*
   * Assert things about the mocks
   */
  tt_int_op(unlinked_count, OP_EQ, 0);
  tt_int_op(write_str_count, OP_EQ, 1);
  tt_mem_op(last_write_str_hash, OP_EQ, test_desc_3_hash, DIGEST_SHA256);

  /* Second time */
  dump_desc(test_desc_4, test_desc_type);

  /*
   * Assert things about the FIFO state
   */
  tt_u64_op(len_descs_dumped, OP_EQ,
            strlen(test_desc_3) + strlen(test_desc_4));
  tt_assert(descs_dumped != NULL && smartlist_len(descs_dumped) == 2);

  /*
   * Assert things about the mocks
   */
  tt_int_op(unlinked_count, OP_EQ, 0);
  tt_int_op(write_str_count, OP_EQ, 2);
  tt_mem_op(last_write_str_hash, OP_EQ, test_desc_4_hash, DIGEST_SHA256);

  /* Third time */
  dump_desc(test_desc_4, test_desc_type);

  /*
   * Assert things about the FIFO state
   */
  tt_u64_op(len_descs_dumped, OP_EQ,
            strlen(test_desc_3) + strlen(test_desc_4));
  tt_assert(descs_dumped != NULL && smartlist_len(descs_dumped) == 2);

  /*
   * Assert things about the mocks
   */
  tt_int_op(unlinked_count, OP_EQ, 0);
  tt_int_op(write_str_count, OP_EQ, 2);
  tt_mem_op(last_write_str_hash, OP_EQ, test_desc_4_hash, DIGEST_SHA256);

  /*
   * Reset the FIFO and check its state
   */
  dump_desc_fifo_cleanup();
  tt_u64_op(len_descs_dumped, OP_EQ, 0);
  tt_assert(descs_dumped == NULL || smartlist_len(descs_dumped) == 0);

  /*
   * Reset the mocks and check their state
   */
  mock_unlink_reset();
  mock_write_str_to_file_reset();
  tt_int_op(unlinked_count, OP_EQ, 0);
  tt_int_op(write_str_count, OP_EQ, 0);

  /*
   * (5) Same for the (A B A) repetition
   */

  /* First time */
  dump_desc(test_desc_1, test_desc_type);

  /*
   * Assert things about the FIFO state
   */
  tt_u64_op(len_descs_dumped, OP_EQ, strlen(test_desc_1));
  tt_assert(descs_dumped != NULL && smartlist_len(descs_dumped) == 1);

  /*
   * Assert things about the mocks
   */
  tt_int_op(unlinked_count, OP_EQ, 0);
  tt_int_op(write_str_count, OP_EQ, 1);
  tt_mem_op(last_write_str_hash, OP_EQ, test_desc_1_hash, DIGEST_SHA256);

  /* Second time */
  dump_desc(test_desc_2, test_desc_type);

  /*
   * Assert things about the FIFO state
   */
  tt_u64_op(len_descs_dumped, OP_EQ,
            strlen(test_desc_1) + strlen(test_desc_2));
  tt_assert(descs_dumped != NULL && smartlist_len(descs_dumped) == 2);

  /*
   * Assert things about the mocks
   */
  tt_int_op(unlinked_count, OP_EQ, 0);
  tt_int_op(write_str_count, OP_EQ, 2);
  tt_mem_op(last_write_str_hash, OP_EQ, test_desc_2_hash, DIGEST_SHA256);

  /* Third time */
  dump_desc(test_desc_1, test_desc_type);

  /*
   * Assert things about the FIFO state
   */
  tt_u64_op(len_descs_dumped, OP_EQ,
            strlen(test_desc_1) + strlen(test_desc_2));
  tt_assert(descs_dumped != NULL && smartlist_len(descs_dumped) == 2);

  /*
   * Assert things about the mocks
   */
  tt_int_op(unlinked_count, OP_EQ, 0);
  tt_int_op(write_str_count, OP_EQ, 2);
  tt_mem_op(last_write_str_hash, OP_EQ, test_desc_2_hash, DIGEST_SHA256);

  /*
   * Reset the FIFO and check its state
   */
  dump_desc_fifo_cleanup();
  tt_u64_op(len_descs_dumped, OP_EQ, 0);
  tt_assert(descs_dumped == NULL || smartlist_len(descs_dumped) == 0);

  /*
   * Reset the mocks and check their state
   */
  mock_unlink_reset();
  mock_write_str_to_file_reset();
  tt_int_op(unlinked_count, OP_EQ, 0);
  tt_int_op(write_str_count, OP_EQ, 0);

  /*
   * (6) (A B B C) triggering overflow on C causes A, not B to be unlinked
   */

  /* First time */
  dump_desc(test_desc_3, test_desc_type);

  /*
   * Assert things about the FIFO state
   */
  tt_u64_op(len_descs_dumped, OP_EQ, strlen(test_desc_3));
  tt_assert(descs_dumped != NULL && smartlist_len(descs_dumped) == 1);

  /*
   * Assert things about the mocks
   */
  tt_int_op(unlinked_count, OP_EQ, 0);
  tt_int_op(write_str_count, OP_EQ, 1);
  tt_mem_op(last_write_str_hash, OP_EQ, test_desc_3_hash, DIGEST_SHA256);

  /* Second time */
  dump_desc(test_desc_4, test_desc_type);

  /*
   * Assert things about the FIFO state
   */
  tt_u64_op(len_descs_dumped, OP_EQ,
            strlen(test_desc_3) + strlen(test_desc_4));
  tt_assert(descs_dumped != NULL && smartlist_len(descs_dumped) == 2);

  /*
   * Assert things about the mocks
   */
  tt_int_op(unlinked_count, OP_EQ, 0);
  tt_int_op(write_str_count, OP_EQ, 2);
  tt_mem_op(last_write_str_hash, OP_EQ, test_desc_4_hash, DIGEST_SHA256);

  /* Third time */
  dump_desc(test_desc_4, test_desc_type);

  /*
   * Assert things about the FIFO state
   */
  tt_u64_op(len_descs_dumped, OP_EQ,
            strlen(test_desc_3) + strlen(test_desc_4));
  tt_assert(descs_dumped != NULL && smartlist_len(descs_dumped) == 2);

  /*
   * Assert things about the mocks
   */
  tt_int_op(unlinked_count, OP_EQ, 0);
  tt_int_op(write_str_count, OP_EQ, 2);
  tt_mem_op(last_write_str_hash, OP_EQ, test_desc_4_hash, DIGEST_SHA256);

  /* Fourth time - we should unlink the dump of test_desc_3 here */
  dump_desc(test_desc_1, test_desc_type);

  /*
   * Assert things about the FIFO state
   */
  tt_u64_op(len_descs_dumped, OP_EQ,
            strlen(test_desc_4) + strlen(test_desc_1));
  tt_assert(descs_dumped != NULL && smartlist_len(descs_dumped) == 2);

  /*
   * Assert things about the mocks
   */
  tt_int_op(unlinked_count, OP_EQ, 1);
  tt_int_op(write_str_count, OP_EQ, 3);
  tt_mem_op(last_write_str_hash, OP_EQ, test_desc_1_hash, DIGEST_SHA256);

  /*
   * Reset the FIFO and check its state
   */
  dump_desc_fifo_cleanup();
  tt_u64_op(len_descs_dumped, OP_EQ, 0);
  tt_assert(descs_dumped == NULL || smartlist_len(descs_dumped) == 0);

  /*
   * Reset the mocks and check their state
   */
  mock_unlink_reset();
  mock_write_str_to_file_reset();
  tt_int_op(unlinked_count, OP_EQ, 0);
  tt_int_op(write_str_count, OP_EQ, 0);

  /*
   * (7) (A B A C) triggering overflow on C causes B, not A to be unlinked
   */

  /* First time */
  dump_desc(test_desc_2, test_desc_type);

  /*
   * Assert things about the FIFO state
   */
  tt_u64_op(len_descs_dumped, OP_EQ, strlen(test_desc_2));
  tt_assert(descs_dumped != NULL && smartlist_len(descs_dumped) == 1);

  /*
   * Assert things about the mocks
   */
  tt_int_op(unlinked_count, OP_EQ, 0);
  tt_int_op(write_str_count, OP_EQ, 1);
  tt_mem_op(last_write_str_hash, OP_EQ, test_desc_2_hash, DIGEST_SHA256);

  /* Second time */
  dump_desc(test_desc_3, test_desc_type);

  /*
   * Assert things about the FIFO state
   */
  tt_u64_op(len_descs_dumped, OP_EQ,
            strlen(test_desc_2) + strlen(test_desc_3));
  tt_assert(descs_dumped != NULL && smartlist_len(descs_dumped) == 2);

  /*
   * Assert things about the mocks
   */
  tt_int_op(unlinked_count, OP_EQ, 0);
  tt_int_op(write_str_count, OP_EQ, 2);
  tt_mem_op(last_write_str_hash, OP_EQ, test_desc_3_hash, DIGEST_SHA256);

  /* Third time */
  dump_desc(test_desc_2, test_desc_type);

  /*
   * Assert things about the FIFO state
   */
  tt_u64_op(len_descs_dumped, OP_EQ,
            strlen(test_desc_2) + strlen(test_desc_3));
  tt_assert(descs_dumped != NULL && smartlist_len(descs_dumped) == 2);

  /*
   * Assert things about the mocks
   */
  tt_int_op(unlinked_count, OP_EQ, 0);
  tt_int_op(write_str_count, OP_EQ, 2);
  tt_mem_op(last_write_str_hash, OP_EQ, test_desc_3_hash, DIGEST_SHA256);

  /* Fourth time - we should unlink the dump of test_desc_3 here */
  dump_desc(test_desc_4, test_desc_type);

  /*
   * Assert things about the FIFO state
   */
  tt_u64_op(len_descs_dumped, OP_EQ,
            strlen(test_desc_2) + strlen(test_desc_4));
  tt_assert(descs_dumped != NULL && smartlist_len(descs_dumped) == 2);

  /*
   * Assert things about the mocks
   */
  tt_int_op(unlinked_count, OP_EQ, 1);
  tt_int_op(write_str_count, OP_EQ, 3);
  tt_mem_op(last_write_str_hash, OP_EQ, test_desc_4_hash, DIGEST_SHA256);

  /*
   * Reset the FIFO and check its state
   */
  dump_desc_fifo_cleanup();
  tt_u64_op(len_descs_dumped, OP_EQ, 0);
  tt_assert(descs_dumped == NULL || smartlist_len(descs_dumped) == 0);

  /*
   * Reset the mocks and check their state
   */
  mock_unlink_reset();
  mock_write_str_to_file_reset();
  tt_int_op(unlinked_count, OP_EQ, 0);
  tt_int_op(write_str_count, OP_EQ, 0);

 done:

  /* Clean up the fifo */
  dump_desc_fifo_cleanup();

  /* Remove mocks */
  UNMOCK(tor_unlink);
  mock_unlink_reset();
  UNMOCK(write_str_to_file);
  mock_write_str_to_file_reset();
  UNMOCK(options_get_dir_fname2_suffix);
  UNMOCK(check_private_dir);
  UNMOCK(get_options);
  tor_free(mock_options);
  mock_options = NULL;

  return;
}

/* Variables for reset_read_file_to_str_mock() */

static int enforce_expected_filename = 0;
static char *expected_filename = NULL;
static char *file_content = NULL;
static size_t file_content_len = 0;
static struct stat file_stat;
static int read_count = 0, read_call_count = 0;

static void
reset_read_file_to_str_mock(void)
{
  tor_free(expected_filename);
  tor_free(file_content);
  file_content_len = 0;
  memset(&file_stat, 0, sizeof(file_stat));
  read_count = 0;
  read_call_count = 0;
}

static char *
read_file_to_str_mock(const char *filename, int flags,
                      struct stat *stat_out) {
  char *result = NULL;

  /* Insist we got a filename */
  tt_ptr_op(filename, OP_NE, NULL);

  /* We ignore flags */
  (void)flags;

  /* Bump the call count */
  ++read_call_count;

  if (enforce_expected_filename) {
    tt_assert(expected_filename);
    tt_str_op(filename, OP_EQ, expected_filename);
  }

  if (expected_filename != NULL &&
      file_content != NULL &&
      strcmp(filename, expected_filename) == 0) {
    /* You asked for it, you got it */

    /*
     * This is the same behavior as the real read_file_to_str();
     * if there's a NUL, the real size ends up in stat_out.
     */
    result = tor_malloc(file_content_len + 1);
    if (file_content_len > 0) {
      memcpy(result, file_content, file_content_len);
    }
    result[file_content_len] = '\0';

    /* Do we need to set up stat_out? */
    if (stat_out != NULL) {
      memcpy(stat_out, &file_stat, sizeof(file_stat));
      /* We always return the correct length here */
      stat_out->st_size = file_content_len;
    }

    /* Wooo, we have a return value - bump the counter */
    ++read_count;
  }
  /* else no match, return NULL */

 done:
  return result;
}

/* This one tests dump_desc_populate_one_file() */
static void
test_dir_populate_dump_desc_fifo(void *data)
{
  const char *dirname = "foo";
  const char *fname = NULL;
  dumped_desc_t *ent;

  (void)data;

  /*
   * Set up unlink and read_file_to_str mocks
   */
  MOCK(tor_unlink, mock_unlink);
  mock_unlink_reset();
  MOCK(read_file_to_str, read_file_to_str_mock);
  reset_read_file_to_str_mock();

  /* Check state of unlink mock */
  tt_int_op(unlinked_count, OP_EQ, 0);

  /* Some cases that should fail before trying to read the file */
  ent = dump_desc_populate_one_file(dirname, "bar");
  tt_ptr_op(ent, OP_EQ, NULL);
  tt_int_op(unlinked_count, OP_EQ, 1);
  tt_int_op(read_count, OP_EQ, 0);
  tt_int_op(read_call_count, OP_EQ, 0);

  ent = dump_desc_populate_one_file(dirname, "unparseable-desc");
  tt_ptr_op(ent, OP_EQ, NULL);
  tt_int_op(unlinked_count, OP_EQ, 2);
  tt_int_op(read_count, OP_EQ, 0);
  tt_int_op(read_call_count, OP_EQ, 0);

  ent = dump_desc_populate_one_file(dirname, "unparseable-desc.baz");
  tt_ptr_op(ent, OP_EQ, NULL);
  tt_int_op(unlinked_count, OP_EQ, 3);
  tt_int_op(read_count, OP_EQ, 0);
  tt_int_op(read_call_count, OP_EQ, 0);

  ent = dump_desc_populate_one_file(
      dirname,
      "unparseable-desc.08AE85E90461F59E");
  tt_ptr_op(ent, OP_EQ, NULL);
  tt_int_op(unlinked_count, OP_EQ, 4);
  tt_int_op(read_count, OP_EQ, 0);
  tt_int_op(read_call_count, OP_EQ, 0);

  ent = dump_desc_populate_one_file(
      dirname,
      "unparseable-desc.08AE85E90461F59EDF0981323F3A70D02B55AB54B44B04F"
      "287D72F7B72F242E85C8CB0EDA8854A99");
  tt_ptr_op(ent, OP_EQ, NULL);
  tt_int_op(unlinked_count, OP_EQ, 5);
  tt_int_op(read_count, OP_EQ, 0);
  tt_int_op(read_call_count, OP_EQ, 0);

  /* This is a correct-length digest but base16_decode() will fail */
  ent = dump_desc_populate_one_file(
      dirname,
      "unparseable-desc.68219B8BGE64B705A6FFC728C069DC596216D60A7D7520C"
      "D5ECE250D912E686B");
  tt_ptr_op(ent, OP_EQ, NULL);
  tt_int_op(unlinked_count, OP_EQ, 6);
  tt_int_op(read_count, OP_EQ, 0);
  tt_int_op(read_call_count, OP_EQ, 0);

  /* This one has a correctly formed filename and should try reading */

  /* Read fails */
  ent = dump_desc_populate_one_file(
      dirname,
      "unparseable-desc.DF0981323F3A70D02B55AB54B44B04F287D72F7B72F242E"
      "85C8CB0EDA8854A99");
  tt_ptr_op(ent, OP_EQ, NULL);
  tt_int_op(unlinked_count, OP_EQ, 7);
  tt_int_op(read_count, OP_EQ, 0);
  tt_int_op(read_call_count, OP_EQ, 1);

  /* This read will succeed but the digest won't match the file content */
  fname =
    "unparseable-desc."
    "DF0981323F3A70D02B55AB54B44B04F287D72F7B72F242E85C8CB0EDA8854A99";
  enforce_expected_filename = 1;
  tor_asprintf(&expected_filename, "%s%s%s", dirname, PATH_SEPARATOR, fname);
  file_content = tor_strdup("hanc culpam maiorem an illam dicam?");
  file_content_len = strlen(file_content);
  file_stat.st_mtime = 123456;
  ent = dump_desc_populate_one_file(dirname, fname);
  enforce_expected_filename = 0;
  tt_ptr_op(ent, OP_EQ, NULL);
  tt_int_op(unlinked_count, OP_EQ, 8);
  tt_int_op(read_count, OP_EQ, 1);
  tt_int_op(read_call_count, OP_EQ, 2);
  tor_free(expected_filename);
  tor_free(file_content);

  /* This one will match */
  fname =
    "unparseable-desc."
    "0786C7173447B7FB033FFCA2FC47C3CF71C30DD47CA8236D3FC7FF35853271C6";
  tor_asprintf(&expected_filename, "%s%s%s", dirname, PATH_SEPARATOR, fname);
  file_content = tor_strdup("hanc culpam maiorem an illam dicam?");
  file_content_len = strlen(file_content);
  file_stat.st_mtime = 789012;
  ent = dump_desc_populate_one_file(dirname, fname);
  tt_ptr_op(ent, OP_NE, NULL);
  tt_int_op(unlinked_count, OP_EQ, 8);
  tt_int_op(read_count, OP_EQ, 2);
  tt_int_op(read_call_count, OP_EQ, 3);
  tt_str_op(ent->filename, OP_EQ, expected_filename);
  tt_int_op(ent->len, OP_EQ, file_content_len);
  tt_int_op(ent->when, OP_EQ, file_stat.st_mtime);
  tor_free(ent->filename);
  tor_free(ent);
  tor_free(expected_filename);

  /*
   * Reset the mocks and check their state
   */
  mock_unlink_reset();
  tt_int_op(unlinked_count, OP_EQ, 0);
  reset_read_file_to_str_mock();
  tt_int_op(read_count, OP_EQ, 0);

 done:

  UNMOCK(tor_unlink);
  mock_unlink_reset();
  UNMOCK(read_file_to_str);
  reset_read_file_to_str_mock();

  tor_free(file_content);

  return;
}

static smartlist_t *
listdir_mock(const char *dname)
{
  smartlist_t *l;

  /* Ignore the name, always return this list */
  (void)dname;

  l = smartlist_new();
  smartlist_add_strdup(l, "foo");
  smartlist_add_strdup(l, "bar");
  smartlist_add_strdup(l, "baz");

  return l;
}

static dumped_desc_t *
pop_one_mock(const char *dirname, const char *f)
{
  dumped_desc_t *ent = NULL;

  if (dirname != NULL && strcmp(dirname, "d") == 0) {
    if (f != NULL && strcmp(f, "foo") == 0) {
      ent = tor_malloc_zero(sizeof(*ent));
      ent->filename = tor_strdup("d/foo");
      ent->len = 123;
      ent->digest_sha256[0] = 1;
      ent->when = 1024;
    } else if (f != NULL && strcmp(f, "bar") == 0) {
      ent = tor_malloc_zero(sizeof(*ent));
      ent->filename = tor_strdup("d/bar");
      ent->len = 456;
      ent->digest_sha256[0] = 2;
      /*
       * Note that the timestamps are in a different order than
       * listdir_mock() returns; we're testing the sort order.
       */
      ent->when = 512;
    } else if (f != NULL && strcmp(f, "baz") == 0) {
      ent = tor_malloc_zero(sizeof(*ent));
      ent->filename = tor_strdup("d/baz");
      ent->len = 789;
      ent->digest_sha256[0] = 3;
      ent->when = 768;
    }
  }

  return ent;
}

/* This one tests dump_desc_populate_fifo_from_directory() */
static void
test_dir_populate_dump_desc_fifo_2(void *data)
{
  dumped_desc_t *ent = NULL;

  (void)data;

  /* Set up the mocks */
  MOCK(tor_listdir, listdir_mock);
  MOCK(dump_desc_populate_one_file, pop_one_mock);

  /* Run dump_desc_populate_fifo_from_directory() */
  descs_dumped = NULL;
  len_descs_dumped = 0;
  dump_desc_populate_fifo_from_directory("d");
  tt_assert(descs_dumped != NULL);
  tt_int_op(smartlist_len(descs_dumped), OP_EQ, 3);
  tt_u64_op(len_descs_dumped, OP_EQ, 1368);
  ent = smartlist_get(descs_dumped, 0);
  tt_str_op(ent->filename, OP_EQ, "d/bar");
  tt_int_op(ent->len, OP_EQ, 456);
  tt_int_op(ent->when, OP_EQ, 512);
  ent = smartlist_get(descs_dumped, 1);
  tt_str_op(ent->filename, OP_EQ, "d/baz");
  tt_int_op(ent->len, OP_EQ, 789);
  tt_int_op(ent->when, OP_EQ, 768);
  ent = smartlist_get(descs_dumped, 2);
  tt_str_op(ent->filename, OP_EQ, "d/foo");
  tt_int_op(ent->len, OP_EQ, 123);
  tt_int_op(ent->when, OP_EQ, 1024);

 done:
  dump_desc_fifo_cleanup();

  UNMOCK(dump_desc_populate_one_file);
  UNMOCK(tor_listdir);

  return;
}

static int mock_networkstatus_consensus_is_bootstrapping_value = 0;
static int
mock_networkstatus_consensus_is_bootstrapping(time_t now)
{
  (void)now;
  return mock_networkstatus_consensus_is_bootstrapping_value;
}

static int mock_networkstatus_consensus_can_use_extra_fallbacks_value = 0;
static int
mock_networkstatus_consensus_can_use_extra_fallbacks(
                                                  const or_options_t *options)
{
  (void)options;
  return mock_networkstatus_consensus_can_use_extra_fallbacks_value;
}

static int mock_num_bridges_usable_value = 0;
static int
mock_num_bridges_usable(int use_maybe_reachable)
{
  (void)use_maybe_reachable;
  return mock_num_bridges_usable_value;
}

/* data is a 3 character nul-terminated string.
 * If data[0] is 'b', set bootstrapping, anything else means not bootstrapping
 * If data[1] is 'f', set extra fallbacks, anything else means no extra
 * If data[2] is 'f', set running bridges, anything else means no extra
 * fallbacks.
 */
static void
test_dir_find_dl_min_delay(void* data)
{
  const char *str = (const char *)data;

  tt_assert(strlen(data) == 3);

  if (str[0] == 'b') {
    mock_networkstatus_consensus_is_bootstrapping_value = 1;
  } else {
    mock_networkstatus_consensus_is_bootstrapping_value = 0;
  }

  if (str[1] == 'f') {
    mock_networkstatus_consensus_can_use_extra_fallbacks_value = 1;
  } else {
    mock_networkstatus_consensus_can_use_extra_fallbacks_value = 0;
  }

  if (str[2] == 'r') {
    /* Any positive, non-zero value should work */
    mock_num_bridges_usable_value = 2;
  } else {
    mock_num_bridges_usable_value = 0;
  }

  MOCK(networkstatus_consensus_is_bootstrapping,
       mock_networkstatus_consensus_is_bootstrapping);
  MOCK(networkstatus_consensus_can_use_extra_fallbacks,
       mock_networkstatus_consensus_can_use_extra_fallbacks);
  MOCK(num_bridges_usable,
       mock_num_bridges_usable);

  download_status_t dls;

  const int server=10, client=20, server_cons=30, client_cons=40;
  const int client_boot_auth_only_cons=50, client_boot_auth_cons=60;
  const int client_boot_fallback_cons=70, bridge=80, bridge_bootstrap=90;

  mock_options = tor_malloc(sizeof(or_options_t));
  reset_options(mock_options, &mock_get_options_calls);
  MOCK(get_options, mock_get_options);

  mock_options->TestingServerDownloadInitialDelay = server;
  mock_options->TestingClientDownloadInitialDelay = client;
  mock_options->TestingServerConsensusDownloadInitialDelay = server_cons;
  mock_options->TestingClientConsensusDownloadInitialDelay = client_cons;
  mock_options->ClientBootstrapConsensusAuthorityOnlyDownloadInitialDelay =
    client_boot_auth_only_cons;
  mock_options->ClientBootstrapConsensusAuthorityDownloadInitialDelay =
    client_boot_auth_cons;
  mock_options->ClientBootstrapConsensusFallbackDownloadInitialDelay =
    client_boot_fallback_cons;
  mock_options->TestingBridgeDownloadInitialDelay = bridge;
  mock_options->TestingBridgeBootstrapDownloadInitialDelay = bridge_bootstrap;

  dls.schedule = DL_SCHED_GENERIC;
  /* client */
  mock_options->ClientOnly = 1;
  tt_int_op(find_dl_min_delay(&dls, mock_options), OP_EQ, client);
  mock_options->ClientOnly = 0;

  /* dir mode */
  mock_options->DirPort_set = 1;
  mock_options->DirCache = 1;
  tt_int_op(find_dl_min_delay(&dls, mock_options), OP_EQ, server);
  mock_options->DirPort_set = 0;
  mock_options->DirCache = 0;

  dls.schedule = DL_SCHED_CONSENSUS;
  /* public server mode */
  mock_options->ORPort_set = 1;
  tt_int_op(find_dl_min_delay(&dls, mock_options), OP_EQ, server_cons);
  mock_options->ORPort_set = 0;

  /* client and bridge modes */
  if (networkstatus_consensus_is_bootstrapping(time(NULL))) {
    if (networkstatus_consensus_can_use_extra_fallbacks(mock_options)) {
      dls.want_authority = 1;
      /* client */
      mock_options->ClientOnly = 1;
      tt_int_op(find_dl_min_delay(&dls, mock_options), OP_EQ,
                client_boot_auth_cons);
      mock_options->ClientOnly = 0;

      /* bridge relay */
      mock_options->ORPort_set = 1;
      mock_options->BridgeRelay = 1;
      tt_int_op(find_dl_min_delay(&dls, mock_options), OP_EQ,
                client_boot_auth_cons);
      mock_options->ORPort_set = 0;
      mock_options->BridgeRelay = 0;

      dls.want_authority = 0;
      /* client */
      mock_options->ClientOnly = 1;
      tt_int_op(find_dl_min_delay(&dls, mock_options), OP_EQ,
                client_boot_fallback_cons);
      mock_options->ClientOnly = 0;

      /* bridge relay */
      mock_options->ORPort_set = 1;
      mock_options->BridgeRelay = 1;
      tt_int_op(find_dl_min_delay(&dls, mock_options), OP_EQ,
                client_boot_fallback_cons);
      mock_options->ORPort_set = 0;
      mock_options->BridgeRelay = 0;

    } else {
      /* dls.want_authority is ignored */
      /* client */
      mock_options->ClientOnly = 1;
      tt_int_op(find_dl_min_delay(&dls, mock_options), OP_EQ,
                client_boot_auth_only_cons);
      mock_options->ClientOnly = 0;

      /* bridge relay */
      mock_options->ORPort_set = 1;
      mock_options->BridgeRelay = 1;
      tt_int_op(find_dl_min_delay(&dls, mock_options), OP_EQ,
                client_boot_auth_only_cons);
      mock_options->ORPort_set = 0;
      mock_options->BridgeRelay = 0;
    }
  } else {
    /* client */
    mock_options->ClientOnly = 1;
    tt_int_op(find_dl_min_delay(&dls, mock_options), OP_EQ,
              client_cons);
    mock_options->ClientOnly = 0;

    /* bridge relay */
    mock_options->ORPort_set = 1;
    mock_options->BridgeRelay = 1;
    tt_int_op(find_dl_min_delay(&dls, mock_options), OP_EQ,
              client_cons);
    mock_options->ORPort_set = 0;
    mock_options->BridgeRelay = 0;
  }

  dls.schedule = DL_SCHED_BRIDGE;
  /* client */
  mock_options->ClientOnly = 1;
  mock_options->UseBridges = 1;
  if (num_bridges_usable(0) > 0) {
    tt_int_op(find_dl_min_delay(&dls, mock_options), OP_EQ, bridge);
  } else {
    tt_int_op(find_dl_min_delay(&dls, mock_options), OP_EQ, bridge_bootstrap);
  }

 done:
  UNMOCK(networkstatus_consensus_is_bootstrapping);
  UNMOCK(networkstatus_consensus_can_use_extra_fallbacks);
  UNMOCK(num_bridges_usable);
  UNMOCK(get_options);
  tor_free(mock_options);
  mock_options = NULL;
}

static void
test_dir_matching_flags(void *arg)
{
  (void) arg;
  routerstatus_t *rs_noflags = NULL;
  routerstatus_t *rs = NULL;
  char *s = NULL;

  smartlist_t *tokens = smartlist_new();
  memarea_t *area = memarea_new();

  int expected_val_when_unused = 0;

  const char *ex_noflags =
    "r example hereiswhereyouridentitygoes 2015-08-30 12:00:00 "
       "192.168.0.1 9001 0\n"
    "m thisoneislongerbecauseitisa256bitmddigest33\n"
    "s\n";
  const char *cp = ex_noflags;
  rs_noflags = routerstatus_parse_entry_from_string(
         area, &cp,
         cp + strlen(cp),
         tokens, NULL, NULL,
         MAX_SUPPORTED_CONSENSUS_METHOD, FLAV_MICRODESC);
  tt_assert(rs_noflags);

#define FLAG(string, field) STMT_BEGIN {        \
    tor_asprintf(&s,\
                 "r example hereiswhereyouridentitygoes 2015-08-30 12:00:00 " \
                 "192.168.0.1 9001 0\n"                                 \
                 "m thisoneislongerbecauseitisa256bitmddigest33\n"      \
                 "s %s\n", string);                                     \
    cp = s;                                                             \
    rs =  routerstatus_parse_entry_from_string(                         \
      area, &cp,                                                        \
      cp + strlen(cp),                                                  \
      tokens, NULL, NULL,                                               \
      MAX_SUPPORTED_CONSENSUS_METHOD, FLAV_MICRODESC);                  \
    /* the field should usually be 0 when no flags are listed */        \
    tt_int_op(rs_noflags->field, OP_EQ, expected_val_when_unused);      \
    /* the field should be 1 when this flags islisted */                \
    tt_int_op(rs->field, OP_EQ, 1);                                     \
    tor_free(s);                                                        \
    routerstatus_free(rs);                                              \
} STMT_END

  FLAG("Authority", is_authority);
  FLAG("BadExit", is_bad_exit);
  FLAG("Exit", is_exit);
  FLAG("Fast", is_fast);
  FLAG("Guard", is_possible_guard);
  FLAG("HSDir", is_hs_dir);
  FLAG("Stable", is_stable);
  FLAG("StaleDesc", is_staledesc);
  FLAG("V2Dir", is_v2_dir);

  // These flags are assumed to be set whether they're declared or not.
  expected_val_when_unused = 1;
  FLAG("Running", is_flagged_running);
  FLAG("Valid", is_valid);
  expected_val_when_unused = 0;

  // These flags are no longer used, but still parsed.
  FLAG("Named", is_named);
  FLAG("Unnamed", is_unnamed);

 done:
  tor_free(s);
  routerstatus_free(rs);
  routerstatus_free(rs_noflags);
  memarea_drop_all(area);
  smartlist_free(tokens);
}

static void
test_dir_assumed_flags(void *arg)
{
  (void)arg;
  smartlist_t *tokens = smartlist_new();
  memarea_t *area = memarea_new();
  routerstatus_t *rs = NULL;

  /* We can assume that consensus method is higher than 24, so Running and
   * Valid are always implicitly set */
  const char *str1 =
    "r example hereiswhereyouridentitygoes 2015-08-30 12:00:00 "
       "192.168.0.1 9001 0\n"
    "m thisoneislongerbecauseitisa256bitmddigest33\n"
    "s Fast Guard Stable\n";
  const char *eos = str1 + strlen(str1);

  const char *cp = str1;
  rs = routerstatus_parse_entry_from_string(area, &cp, eos, tokens, NULL, NULL,
                                            24, FLAV_MICRODESC);
  tt_assert(rs);
  tt_assert(rs->is_flagged_running);
  tt_assert(rs->is_valid);
  tt_assert(! rs->is_exit);
  tt_assert(rs->is_fast);

 done:
  smartlist_free(tokens);
  memarea_drop_all(area);
  routerstatus_free(rs);
}

static void
test_dir_post_parsing(void *arg)
{
  (void) arg;

  /* Test the version parsing from an HS descriptor publish request. */
  {
    const char *end;
    const char *prefix = "/tor/hs/";
    int version = parse_hs_version_from_post("/tor/hs//publish", prefix, &end);
    tt_int_op(version, OP_EQ, -1);
    tt_ptr_op(end, OP_EQ, NULL);
    version = parse_hs_version_from_post("/tor/hs/a/publish", prefix, &end);
    tt_int_op(version, OP_EQ, -1);
    tt_ptr_op(end, OP_EQ, NULL);
    version = parse_hs_version_from_post("/tor/hs/3/publish", prefix, &end);
    tt_int_op(version, OP_EQ, 3);
    tt_str_op(end, OP_EQ, "/publish");
    version = parse_hs_version_from_post("/tor/hs/42/publish", prefix, &end);
    tt_int_op(version, OP_EQ, 42);
    tt_str_op(end, OP_EQ, "/publish");
    version = parse_hs_version_from_post("/tor/hs/18163/publish",prefix, &end);
    tt_int_op(version, OP_EQ, 18163);
    tt_str_op(end, OP_EQ, "/publish");
    version = parse_hs_version_from_post("JUNKJUNKJUNK", prefix, &end);
    tt_int_op(version, OP_EQ, -1);
    tt_ptr_op(end, OP_EQ, NULL);
    version = parse_hs_version_from_post("/tor/hs/3/publish", "blah", &end);
    tt_int_op(version, OP_EQ, -1);
    tt_ptr_op(end, OP_EQ, NULL);
    /* Missing the '/' at the end of the prefix. */
    version = parse_hs_version_from_post("/tor/hs/3/publish", "/tor/hs", &end);
    tt_int_op(version, OP_EQ, -1);
    tt_ptr_op(end, OP_EQ, NULL);
    version = parse_hs_version_from_post("/random/blah/tor/hs/3/publish",
                                         prefix, &end);
    tt_int_op(version, OP_EQ, -1);
    tt_ptr_op(end, OP_EQ, NULL);
    version = parse_hs_version_from_post("/tor/hs/3/publish/random/junk",
                                         prefix, &end);
    tt_int_op(version, OP_EQ, 3);
    tt_str_op(end, OP_EQ, "/publish/random/junk");
    version = parse_hs_version_from_post("/tor/hs/-1/publish", prefix, &end);
    tt_int_op(version, OP_EQ, -1);
    tt_ptr_op(end, OP_EQ, NULL);
    /* INT_MAX */
    version = parse_hs_version_from_post("/tor/hs/2147483647/publish",
                                         prefix, &end);
    tt_int_op(version, OP_EQ, INT_MAX);
    tt_str_op(end, OP_EQ, "/publish");
    /* INT_MAX + 1*/
    version = parse_hs_version_from_post("/tor/hs/2147483648/publish",
                                         prefix, &end);
    tt_int_op(version, OP_EQ, -1);
    tt_ptr_op(end, OP_EQ, NULL);
  }

 done:
  ;
}

static void
test_dir_platform_str(void *arg)
{
  char platform[256];
  (void)arg;
  platform[0] = 0;
  get_platform_str(platform, sizeof(platform));
  tt_int_op((int)strlen(platform), OP_GT, 0);
  tt_assert(!strcmpstart(platform, "Tor "));

  tor_version_t ver;
  // make sure this is a tor version, a real actual tor version.
  tt_int_op(tor_version_parse_platform(platform, &ver, 1), OP_EQ, 1);

  TT_BLATHER(("%d.%d.%d.%d", ver.major, ver.minor, ver.micro, ver.patchlevel));

  // Handle an example version.
  tt_int_op(tor_version_parse_platform(
        "Tor 0.3.3.3 (foo) (git-xyzzy) on a potato", &ver, 1), OP_EQ, 1);
 done:
  ;
}

static networkstatus_t *mock_networkstatus;

static networkstatus_t *
mock_networkstatus_get_latest_consensus_by_flavor(consensus_flavor_t f)
{
  (void)f;
  return mock_networkstatus;
}

static void
test_dir_networkstatus_consensus_has_ipv6(void *arg)
{
  (void)arg;

  int has_ipv6 = 0;

  /* Init options and networkstatus */
  or_options_t our_options;
  mock_options = &our_options;
  reset_options(mock_options, &mock_get_options_calls);
  MOCK(get_options, mock_get_options);

  networkstatus_t our_networkstatus;
  mock_networkstatus = &our_networkstatus;
  memset(mock_networkstatus, 0, sizeof(*mock_networkstatus));
  MOCK(networkstatus_get_latest_consensus_by_flavor,
       mock_networkstatus_get_latest_consensus_by_flavor);

  /* A live consensus */
  mock_networkstatus->valid_after = time(NULL) - 3600;
  mock_networkstatus->valid_until = time(NULL) + 3600;

  /* Test the bounds for A lines in the NS consensus */
  mock_options->UseMicrodescriptors = 0;

  mock_networkstatus->consensus_method = MIN_SUPPORTED_CONSENSUS_METHOD;
  has_ipv6 = networkstatus_consensus_has_ipv6(get_options());
  tt_assert(has_ipv6);

  /* Test the bounds for A lines in the microdesc consensus */
  mock_options->UseMicrodescriptors = 1;

  mock_networkstatus->consensus_method =
      MIN_METHOD_FOR_A_LINES_IN_MICRODESC_CONSENSUS;
  has_ipv6 = networkstatus_consensus_has_ipv6(get_options());
  tt_assert(has_ipv6);

  mock_networkstatus->consensus_method = MAX_SUPPORTED_CONSENSUS_METHOD + 20;
  has_ipv6 = networkstatus_consensus_has_ipv6(get_options());
  tt_assert(has_ipv6);

  mock_networkstatus->consensus_method =
      MIN_METHOD_FOR_A_LINES_IN_MICRODESC_CONSENSUS + 1;
  has_ipv6 = networkstatus_consensus_has_ipv6(get_options());
  tt_assert(has_ipv6);

  mock_networkstatus->consensus_method =
      MIN_METHOD_FOR_A_LINES_IN_MICRODESC_CONSENSUS + 20;
  has_ipv6 = networkstatus_consensus_has_ipv6(get_options());
  tt_assert(has_ipv6);

  mock_networkstatus->consensus_method =
      MIN_METHOD_FOR_A_LINES_IN_MICRODESC_CONSENSUS - 1;
  has_ipv6 = networkstatus_consensus_has_ipv6(get_options());
  tt_assert(!has_ipv6);

  /* Test the edge cases */
  mock_options->UseMicrodescriptors = 1;
  mock_networkstatus->consensus_method =
      MIN_METHOD_FOR_A_LINES_IN_MICRODESC_CONSENSUS;

  /* Reasonably live */
  mock_networkstatus->valid_until = approx_time() - 60;
  has_ipv6 = networkstatus_consensus_has_ipv6(get_options());
  tt_assert(has_ipv6);

  /* Not reasonably live */
  mock_networkstatus->valid_after = approx_time() - 24*60*60 - 3600;
  mock_networkstatus->valid_until = approx_time() - 24*60*60 - 60;
  has_ipv6 = networkstatus_consensus_has_ipv6(get_options());
  tt_assert(!has_ipv6);

  /* NULL consensus */
  mock_networkstatus = NULL;
  has_ipv6 = networkstatus_consensus_has_ipv6(get_options());
  tt_assert(!has_ipv6);

 done:
  UNMOCK(get_options);
  UNMOCK(networkstatus_get_latest_consensus_by_flavor);
}

static void
test_dir_format_versions_list(void *arg)
{
  (void)arg;
  char *s = NULL;
  config_line_t *lines = NULL;

  setup_capture_of_logs(LOG_WARN);
  s = format_recommended_version_list(lines, 1);
  tt_str_op(s, OP_EQ, "");

  tor_free(s);
  config_line_append(&lines, "ignored", "0.3.4.1, 0.2.9.111-alpha, 4.4.4-rc");
  s = format_recommended_version_list(lines, 1);
  tt_str_op(s, OP_EQ,  "0.2.9.111-alpha,0.3.4.1,4.4.4-rc");

  tor_free(s);
  config_line_append(&lines, "ignored", "0.1.2.3,0.2.9.10   ");
  s = format_recommended_version_list(lines, 1);
  tt_str_op(s, OP_EQ,  "0.1.2.3,0.2.9.10,0.2.9.111-alpha,0.3.4.1,4.4.4-rc");

  /* There should be no warnings so far. */
  expect_no_log_entry();

  /* Now try a line with a space in it. */
  tor_free(s);
  config_line_append(&lines, "ignored", "1.3.3.8 1.3.3.7");
  s = format_recommended_version_list(lines, 1);
  tt_str_op(s, OP_EQ,  "0.1.2.3,0.2.9.10,0.2.9.111-alpha,0.3.4.1,"
            "1.3.3.7,1.3.3.8,4.4.4-rc");

  expect_single_log_msg_containing(
          "Unexpected space in versions list member \"1.3.3.8 1.3.3.7\"." );

  /* Start over, with a line containing a bogus version */
  config_free_lines(lines);
  lines = NULL;
  tor_free(s);
  mock_clean_saved_logs();
  config_line_append(&lines, "ignored", "0.1.2.3, alpha-complex, 0.1.1.8-rc");
  s = format_recommended_version_list(lines,1);
  tt_str_op(s, OP_EQ, "0.1.1.8-rc,0.1.2.3,alpha-complex");
  expect_single_log_msg_containing(
        "Recommended version \"alpha-complex\" does not look valid.");

 done:
  tor_free(s);
  config_free_lines(lines);
  teardown_capture_of_logs();
}

#define DIR_LEGACY(name)                             \
  { #name, test_dir_ ## name , TT_FORK, NULL, NULL }

#define DIR(name,flags)                              \
  { #name, test_dir_##name, (flags), NULL, NULL }

/* where arg is a string constant */
#define DIR_ARG(name,flags,arg)                      \
  { #name "_" arg, test_dir_##name, (flags), &passthrough_setup, (void*) arg }

struct testcase_t dir_tests[] = {
  DIR_LEGACY(nicknames),
  /* extrainfo without any stats */
  DIR_ARG(formats_rsa, TT_FORK, ""),
  DIR_ARG(formats_rsa_ed25519, TT_FORK, ""),
  /* on a bridge */
  DIR_ARG(formats_rsa, TT_FORK, "b"),
  DIR_ARG(formats_rsa_ed25519, TT_FORK, "b"),
  /* extrainfo with basic stats */
  DIR_ARG(formats_rsa, TT_FORK, "e"),
  DIR_ARG(formats_rsa_ed25519, TT_FORK, "e"),
  DIR_ARG(formats_rsa, TT_FORK, "be"),
  DIR_ARG(formats_rsa_ed25519, TT_FORK, "be"),
  /* extrainfo with all stats */
  DIR_ARG(formats_rsa, TT_FORK, "es"),
  DIR_ARG(formats_rsa_ed25519, TT_FORK, "es"),
  DIR_ARG(formats_rsa, TT_FORK, "bes"),
  DIR_ARG(formats_rsa_ed25519, TT_FORK, "bes"),
  DIR(routerinfo_parsing, 0),
  DIR(extrainfo_parsing, 0),
  DIR(parse_router_list, TT_FORK),
  DIR(load_routers, TT_FORK),
  DIR(load_extrainfo, TT_FORK),
  DIR(getinfo_extra, 0),
  DIR_LEGACY(versions),
  DIR_LEGACY(fp_pairs),
  DIR(split_fps, 0),
  DIR_LEGACY(measured_bw_kb),
  DIR_LEGACY(measured_bw_kb_line_is_after_headers),
  DIR_LEGACY(measured_bw_kb_cache),
  DIR_LEGACY(dirserv_read_measured_bandwidths),
  DIR(bwauth_bw_file_digest256, 0),
  DIR_LEGACY(param_voting),
  DIR(param_voting_lookup, 0),
  DIR_LEGACY(v3_networkstatus),
  DIR(random_weighted, 0),
  DIR(scale_bw, 0),
  DIR_LEGACY(clip_unmeasured_bw_kb),
  DIR_LEGACY(clip_unmeasured_bw_kb_alt),
  DIR(fmt_control_ns, 0),
  DIR(dirserv_set_routerstatus_testing, 0),
  DIR(http_handling, 0),
  DIR(purpose_needs_anonymity_returns_true_for_bridges, 0),
  DIR(purpose_needs_anonymity_returns_false_for_own_bridge_desc, 0),
  DIR(purpose_needs_anonymity_returns_true_by_default, 0),
  DIR(purpose_needs_anonymity_returns_true_for_sensitive_purpose, 0),
  DIR(purpose_needs_anonymity_ret_false_for_non_sensitive_conn, 0),
  DIR(post_parsing, 0),
  DIR(fetch_type, 0),
  DIR(packages, 0),
  DIR(download_status_random_backoff, 0),
  DIR(download_status_random_backoff_ranges, 0),
  DIR(download_status_increment, TT_FORK),
  DIR(authdir_type_to_string, 0),
  DIR(conn_purpose_to_string, 0),
  DIR(should_use_directory_guards, 0),
  DIR(should_not_init_request_to_ourselves, TT_FORK),
  DIR(should_not_init_request_to_dir_auths_without_v3_info, 0),
  DIR(should_init_request_to_dir_auths, 0),
  DIR(choose_compression_level, 0),
  DIR(dump_unparseable_descriptors, 0),
  DIR(populate_dump_desc_fifo, 0),
  DIR(populate_dump_desc_fifo_2, 0),
  DIR_ARG(find_dl_min_delay, TT_FORK, "bfd"),
  DIR_ARG(find_dl_min_delay, TT_FORK, "bad"),
  DIR_ARG(find_dl_min_delay, TT_FORK, "cfd"),
  DIR_ARG(find_dl_min_delay, TT_FORK, "cad"),
  DIR_ARG(find_dl_min_delay, TT_FORK, "bfr"),
  DIR_ARG(find_dl_min_delay, TT_FORK, "bar"),
  DIR_ARG(find_dl_min_delay, TT_FORK, "cfr"),
  DIR_ARG(find_dl_min_delay, TT_FORK, "car"),
  DIR(assumed_flags, 0),
  DIR(matching_flags, 0),
  DIR(networkstatus_compute_bw_weights_v10, 0),
  DIR(platform_str, 0),
  DIR(networkstatus_consensus_has_ipv6, TT_FORK),
  DIR(format_versions_list, TT_FORK),
  END_OF_TESTCASES
};
