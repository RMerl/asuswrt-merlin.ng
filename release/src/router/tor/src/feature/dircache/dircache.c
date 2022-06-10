/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file dircache.c
 * @brief Cache directories and serve them to clients.
 **/

#define DIRCACHE_PRIVATE

#include "core/or/or.h"

#include "app/config/config.h"
#include "app/config/resolve_addr.h"
#include "core/mainloop/connection.h"
#include "core/or/relay.h"
#include "feature/dirauth/dirvote.h"
#include "feature/dirauth/authmode.h"
#include "feature/dirauth/process_descs.h"
#include "feature/dircache/conscache.h"
#include "feature/dircache/consdiffmgr.h"
#include "feature/dircache/dircache.h"
#include "feature/dircache/dirserv.h"
#include "feature/dircommon/directory.h"
#include "feature/dircommon/fp_pair.h"
#include "feature/hs/hs_cache.h"
#include "feature/nodelist/authcert.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/routerlist.h"
#include "feature/relay/relay_config.h"
#include "feature/relay/routermode.h"
#include "feature/stats/geoip_stats.h"
#include "feature/stats/rephist.h"
#include "lib/compress/compress.h"

#include "feature/dircache/cached_dir_st.h"
#include "feature/dircommon/dir_connection_st.h"
#include "feature/nodelist/authority_cert_st.h"
#include "feature/nodelist/networkstatus_st.h"
#include "feature/nodelist/routerinfo_st.h"

/** Maximum size, in bytes, for any directory object that we're accepting
 * as an upload. */
#define MAX_DIR_UL_SIZE ((1<<24)-1) /* 16MB-1 */

/** HTTP cache control: how long do we tell proxies they can cache each
 * kind of document we serve? */
#define FULL_DIR_CACHE_LIFETIME (60*60)
#define RUNNINGROUTERS_CACHE_LIFETIME (20*60)
#define DIRPORTFRONTPAGE_CACHE_LIFETIME (20*60)
#define NETWORKSTATUS_CACHE_LIFETIME (5*60)
#define ROUTERDESC_CACHE_LIFETIME (30*60)
#define ROUTERDESC_BY_DIGEST_CACHE_LIFETIME (48*60*60)
#define ROBOTS_CACHE_LIFETIME (24*60*60)
#define MICRODESC_CACHE_LIFETIME (48*60*60)
/* Bandwidth files change every hour. */
#define BANDWIDTH_CACHE_LIFETIME (30*60)
/** Parse an HTTP request string <b>headers</b> of the form
 * \verbatim
 * "\%s [http[s]://]\%s HTTP/1..."
 * \endverbatim
 * If it's well-formed, strdup the second \%s into *<b>url</b>, and
 * nul-terminate it. If the url doesn't start with "/tor/", rewrite it
 * so it does. Return 0.
 * Otherwise, return -1.
 */
STATIC int
parse_http_url(const char *headers, char **url)
{
  char *command = NULL;
  if (parse_http_command(headers, &command, url) < 0) {
    return -1;
  }
  if (strcmpstart(*url, "/tor/")) {
    char *new_url = NULL;
    tor_asprintf(&new_url, "/tor%s%s",
                 *url[0] == '/' ? "" : "/",
                 *url);
    tor_free(*url);
    *url = new_url;
  }
  tor_free(command);
  return 0;
}

/** Create an http response for the client <b>conn</b> out of
 * <b>status</b> and <b>reason_phrase</b>. Write it to <b>conn</b>.
 */
static void
write_short_http_response(dir_connection_t *conn, int status,
                       const char *reason_phrase)
{
  char *buf = NULL;
  char *datestring = NULL;

  IF_BUG_ONCE(!reason_phrase) { /* bullet-proofing */
    reason_phrase = "unspecified";
  }

  if (server_mode(get_options())) {
    /* include the Date: header, but only if we're a relay or bridge */
    char datebuf[RFC1123_TIME_LEN+1];
    format_rfc1123_time(datebuf, time(NULL));
    tor_asprintf(&datestring, "Date: %s\r\n", datebuf);
  }

  tor_asprintf(&buf, "HTTP/1.0 %d %s\r\n%s\r\n",
               status, reason_phrase, datestring?datestring:"");

  log_debug(LD_DIRSERV,"Wrote status 'HTTP/1.0 %d %s'", status, reason_phrase);
  connection_buf_add(buf, strlen(buf), TO_CONN(conn));

  tor_free(datestring);
  tor_free(buf);
}

/** Write the header for an HTTP/1.0 response onto <b>conn</b>-\>outbuf,
 * with <b>type</b> as the Content-Type.
 *
 * If <b>length</b> is nonnegative, it is the Content-Length.
 * If <b>encoding</b> is provided, it is the Content-Encoding.
 * If <b>cache_lifetime</b> is greater than 0, the content may be cached for
 * up to cache_lifetime seconds.  Otherwise, the content may not be cached. */
static void
write_http_response_header_impl(dir_connection_t *conn, ssize_t length,
                           const char *type, const char *encoding,
                           const char *extra_headers,
                           long cache_lifetime)
{
  char date[RFC1123_TIME_LEN+1];
  time_t now = approx_time();
  buf_t *buf = buf_new_with_capacity(1024);

  tor_assert(conn);

  format_rfc1123_time(date, now);

  buf_add_printf(buf, "HTTP/1.0 200 OK\r\nDate: %s\r\n", date);
  if (type) {
    buf_add_printf(buf, "Content-Type: %s\r\n", type);
  }
  if (!is_local_to_resolve_addr(&conn->base_.addr)) {
    /* Don't report the source address for a nearby/private connection.
     * Otherwise we tend to mis-report in cases where incoming ports are
     * being forwarded to a Tor server running behind the firewall. */
    buf_add_printf(buf, X_ADDRESS_HEADER "%s\r\n", conn->base_.address);
  }
  if (encoding) {
    buf_add_printf(buf, "Content-Encoding: %s\r\n", encoding);
  }
  if (length >= 0) {
    buf_add_printf(buf, "Content-Length: %ld\r\n", (long)length);
  }
  if (cache_lifetime > 0) {
    char expbuf[RFC1123_TIME_LEN+1];
    format_rfc1123_time(expbuf, (time_t)(now + cache_lifetime));
    /* We could say 'Cache-control: max-age=%d' here if we start doing
     * http/1.1 */
    buf_add_printf(buf, "Expires: %s\r\n", expbuf);
  } else if (cache_lifetime == 0) {
    /* We could say 'Cache-control: no-cache' here if we start doing
     * http/1.1 */
    buf_add_string(buf, "Pragma: no-cache\r\n");
  }
  if (extra_headers) {
    buf_add_string(buf, extra_headers);
  }
  buf_add_string(buf, "\r\n");

  connection_buf_add_buf(TO_CONN(conn), buf);
  buf_free(buf);
}

/** As write_http_response_header_impl, but translates method into
 * encoding */
static void
write_http_response_headers(dir_connection_t *conn, ssize_t length,
                            compress_method_t method,
                            const char *extra_headers, long cache_lifetime)
{
  write_http_response_header_impl(conn, length,
                                  "text/plain",
                                  compression_method_get_name(method),
                                  extra_headers,
                                  cache_lifetime);
}

/** As write_http_response_headers, but assumes extra_headers is NULL */
static void
write_http_response_header(dir_connection_t *conn, ssize_t length,
                           compress_method_t method,
                           long cache_lifetime)
{
  write_http_response_headers(conn, length, method, NULL, cache_lifetime);
}

/** Array of compression methods to use (if supported) for serving
 * precompressed data, ordered from best to worst. */
static compress_method_t srv_meth_pref_precompressed[] = {
  LZMA_METHOD,
  ZSTD_METHOD,
  ZLIB_METHOD,
  GZIP_METHOD,
  NO_METHOD
};

/** Array of compression methods to use (if supported) for serving
 * streamed data, ordered from best to worst. */
static compress_method_t srv_meth_pref_streaming_compression[] = {
  ZSTD_METHOD,
  ZLIB_METHOD,
  GZIP_METHOD,
  NO_METHOD
};

/** Parse the compression methods listed in an Accept-Encoding header <b>h</b>,
 * and convert them to a bitfield where compression method x is supported if
 * and only if 1 &lt;&lt; x is set in the bitfield. */
STATIC unsigned
parse_accept_encoding_header(const char *h)
{
  unsigned result = (1u << NO_METHOD);
  smartlist_t *methods = smartlist_new();
  smartlist_split_string(methods, h, ",",
             SPLIT_SKIP_SPACE|SPLIT_STRIP_SPACE|SPLIT_IGNORE_BLANK, 0);

  SMARTLIST_FOREACH_BEGIN(methods, const char *, m) {
    compress_method_t method = compression_method_get_by_name(m);
    if (method != UNKNOWN_METHOD) {
      tor_assert(((unsigned)method) < 8*sizeof(unsigned));
      result |= (1u << method);
    }
  } SMARTLIST_FOREACH_END(m);
  SMARTLIST_FOREACH_BEGIN(methods, char *, m) {
    tor_free(m);
  } SMARTLIST_FOREACH_END(m);
  smartlist_free(methods);
  return result;
}

/** Decide whether a client would accept the consensus we have.
 *
 * Clients can say they only want a consensus if it's signed by more
 * than half the authorities in a list.  They pass this list in
 * the url as "...consensus/<b>fpr</b>+<b>fpr</b>+<b>fpr</b>".
 *
 * <b>fpr</b> may be an abbreviated fingerprint, i.e. only a left substring
 * of the full authority identity digest. (Only strings of even length,
 * i.e. encodings of full bytes, are handled correctly.  In the case
 * of an odd number of hex digits the last one is silently ignored.)
 *
 * Returns 1 if more than half of the requested authorities signed the
 * consensus, 0 otherwise.
 */
static int
client_likes_consensus(const struct consensus_cache_entry_t *ent,
                       const char *want_url)
{
  smartlist_t *voters = smartlist_new();
  int need_at_least;
  int have = 0;

  if (consensus_cache_entry_get_voter_id_digests(ent, voters) != 0) {
    smartlist_free(voters);
    return 1; // We don't know the voters; assume the client won't mind. */
  }

  smartlist_t *want_authorities = smartlist_new();
  dir_split_resource_into_fingerprints(want_url, want_authorities, NULL, 0);
  need_at_least = smartlist_len(want_authorities)/2+1;

  SMARTLIST_FOREACH_BEGIN(want_authorities, const char *, want_digest) {

    SMARTLIST_FOREACH_BEGIN(voters, const char *, digest) {
      if (!strcasecmpstart(digest, want_digest)) {
        have++;
        break;
      };
    } SMARTLIST_FOREACH_END(digest);

    /* early exit, if we already have enough */
    if (have >= need_at_least)
      break;
  } SMARTLIST_FOREACH_END(want_digest);

  SMARTLIST_FOREACH(want_authorities, char *, d, tor_free(d));
  smartlist_free(want_authorities);
  SMARTLIST_FOREACH(voters, char *, cp, tor_free(cp));
  smartlist_free(voters);
  return (have >= need_at_least);
}

/** Return the compression level we should use for sending a compressed
 * response of size <b>n_bytes</b>. */
STATIC compression_level_t
choose_compression_level(void)
{
  /* This is the compression level choice for a stream.
   *
   * We always return LOW because this compression is done in the main thread
   * thus we save CPU time as much as possible, and it is also done more than
   * background compression for document we serve pre-compressed.
   *
   * GZip highest compression level (9) gives us a ratio of 49.72%
   * Zstd lowest compression level (1) gives us a ratio of 47.38%
   *
   * Thus, as the network moves more and more to use Zstd when requesting
   * directory documents that are not pre-cached, even at the
   * lowest level, we still gain over GZip and thus help with load and CPU
   * time on the network. */
  return LOW_COMPRESSION;
}

/** Information passed to handle a GET request. */
typedef struct get_handler_args_t {
  /** Bitmask of compression methods that the client said (or implied) it
   * supported. */
  unsigned compression_supported;
  /** If nonzero, the time included an if-modified-since header with this
   * value. */
  time_t if_modified_since;
  /** String containing the requested URL or resource. */
  const char *url;
  /** String containing the HTTP headers */
  const char *headers;
} get_handler_args_t;

/** Entry for handling an HTTP GET request.
 *
 * This entry matches a request if "string" is equal to the requested
 * resource, or if "is_prefix" is true and "string" is a prefix of the
 * requested resource.
 *
 * The 'handler' function is called to handle the request.  It receives
 * an arguments structure, and must return 0 on success or -1 if we should
 * close the connection.
 **/
typedef struct url_table_ent_t {
  const char *string;
  int is_prefix;
  int (*handler)(dir_connection_t *conn, const get_handler_args_t *args);
} url_table_ent_t;

static int handle_get_frontpage(dir_connection_t *conn,
                                const get_handler_args_t *args);
static int handle_get_current_consensus(dir_connection_t *conn,
                                const get_handler_args_t *args);
static int handle_get_status_vote(dir_connection_t *conn,
                                const get_handler_args_t *args);
static int handle_get_microdesc(dir_connection_t *conn,
                                const get_handler_args_t *args);
static int handle_get_descriptor(dir_connection_t *conn,
                                const get_handler_args_t *args);
static int handle_get_keys(dir_connection_t *conn,
                                const get_handler_args_t *args);
static int handle_get_robots(dir_connection_t *conn,
                                const get_handler_args_t *args);
static int handle_get_networkstatus_bridges(dir_connection_t *conn,
                                const get_handler_args_t *args);
static int handle_get_next_bandwidth(dir_connection_t *conn,
                                     const get_handler_args_t *args);

/** Table for handling GET requests. */
static const url_table_ent_t url_table[] = {
  { "/tor/", 0, handle_get_frontpage },
  { "/tor/status-vote/current/consensus", 1, handle_get_current_consensus },
  { "/tor/status-vote/current/", 1, handle_get_status_vote },
  { "/tor/status-vote/next/bandwidth", 0, handle_get_next_bandwidth },
  { "/tor/status-vote/next/", 1, handle_get_status_vote },
  { "/tor/micro/d/", 1, handle_get_microdesc },
  { "/tor/server/", 1, handle_get_descriptor },
  { "/tor/extra/", 1, handle_get_descriptor },
  { "/tor/keys/", 1, handle_get_keys },
  { "/tor/hs/3/", 1, handle_get_hs_descriptor_v3 },
  { "/tor/robots.txt", 0, handle_get_robots },
  { "/tor/networkstatus-bridges", 0, handle_get_networkstatus_bridges },
  { NULL, 0, NULL },
};

/** Helper function: called when a dirserver gets a complete HTTP GET
 * request.  Look for a request for a directory or for a rendezvous
 * service descriptor.  On finding one, write a response into
 * conn-\>outbuf.  If the request is unrecognized, send a 404.
 * Return 0 if we handled this successfully, or -1 if we need to close
 * the connection. */
MOCK_IMPL(STATIC int,
directory_handle_command_get,(dir_connection_t *conn, const char *headers,
                              const char *req_body, size_t req_body_len))
{
  char *url, *url_mem, *header;
  time_t if_modified_since = 0;
  int zlib_compressed_in_url;
  unsigned compression_methods_supported;

  /* We ignore the body of a GET request. */
  (void)req_body;
  (void)req_body_len;

  log_debug(LD_DIRSERV,"Received GET command.");

  conn->base_.state = DIR_CONN_STATE_SERVER_WRITING;

  if (parse_http_url(headers, &url) < 0) {
    write_short_http_response(conn, 400, "Bad request");
    return 0;
  }
  if ((header = http_get_header(headers, "If-Modified-Since: "))) {
    struct tm tm;
    if (parse_http_time(header, &tm) == 0) {
      if (tor_timegm(&tm, &if_modified_since)<0) {
        if_modified_since = 0;
      } else {
        log_debug(LD_DIRSERV, "If-Modified-Since is '%s'.", escaped(header));
      }
    }
    /* The correct behavior on a malformed If-Modified-Since header is to
     * act as if no If-Modified-Since header had been given. */
    tor_free(header);
  }
  log_debug(LD_DIRSERV,"rewritten url as '%s'.", escaped(url));

  url_mem = url;
  {
    size_t url_len = strlen(url);

    zlib_compressed_in_url = url_len > 2 && !strcmp(url+url_len-2, ".z");
    if (zlib_compressed_in_url) {
      url[url_len-2] = '\0';
    }
  }

  if ((header = http_get_header(headers, "Accept-Encoding: "))) {
    compression_methods_supported = parse_accept_encoding_header(header);
    tor_free(header);
  } else {
    compression_methods_supported = (1u << NO_METHOD);
  }
  if (zlib_compressed_in_url) {
    compression_methods_supported |= (1u << ZLIB_METHOD);
  }

  /* Remove all methods that we don't both support. */
  compression_methods_supported &= tor_compress_get_supported_method_bitmask();

  get_handler_args_t args;
  args.url = url;
  args.headers = headers;
  args.if_modified_since = if_modified_since;
  args.compression_supported = compression_methods_supported;

  int i, result = -1;
  for (i = 0; url_table[i].string; ++i) {
    int match;
    if (url_table[i].is_prefix) {
      match = !strcmpstart(url, url_table[i].string);
    } else {
      match = !strcmp(url, url_table[i].string);
    }
    if (match) {
      result = url_table[i].handler(conn, &args);
      goto done;
    }
  }

  /* we didn't recognize the url */
  write_short_http_response(conn, 404, "Not found");
  result = 0;

 done:
  tor_free(url_mem);
  return result;
}

/** Helper function for GET / or GET /tor/
 */
static int
handle_get_frontpage(dir_connection_t *conn, const get_handler_args_t *args)
{
  (void) args; /* unused */
  const char *frontpage = relay_get_dirportfrontpage();

  if (frontpage) {
    size_t dlen;
    dlen = strlen(frontpage);
    /* Let's return a disclaimer page (users shouldn't use V1 anymore,
       and caches don't fetch '/', so this is safe). */

    /* [We don't check for write_bucket_low here, since we want to serve
     *  this page no matter what.] */
    write_http_response_header_impl(conn, dlen, "text/html", "identity",
                                    NULL, DIRPORTFRONTPAGE_CACHE_LIFETIME);
    connection_buf_add(frontpage, dlen, TO_CONN(conn));
  } else {
    write_short_http_response(conn, 404, "Not found");
  }
  return 0;
}

/** Warn that the cached consensus <b>consensus</b> of type
 * <b>flavor</b> too new or too old, based on <b>is_too_new</b>,
 * and will not be served to clients. Rate-limit the warning to avoid logging
 * an entry on every request.
 */
static void
warn_consensus_is_not_reasonably_live(
                          const struct consensus_cache_entry_t *consensus,
                          const char *flavor, time_t now, bool is_too_new)
{
#define NOT_REASONABLY_LIVE_WARNING_INTERVAL (60*60)
  static ratelim_t warned[2] = { RATELIM_INIT(
                                      NOT_REASONABLY_LIVE_WARNING_INTERVAL),
                                RATELIM_INIT(
                                      NOT_REASONABLY_LIVE_WARNING_INTERVAL) };
  char timestamp[ISO_TIME_LEN+1];
  /* valid_after if is_too_new, valid_until if !is_too_new */
  time_t valid_time = 0;
  char *dupes = NULL;

  if (is_too_new) {
    if (consensus_cache_entry_get_valid_after(consensus, &valid_time))
      return;
    dupes = rate_limit_log(&warned[1], now);
  } else {
    if (consensus_cache_entry_get_valid_until(consensus, &valid_time))
      return;
    dupes = rate_limit_log(&warned[0], now);
  }

  if (dupes) {
    format_local_iso_time(timestamp, valid_time);
    log_warn(LD_DIRSERV, "Our %s%sconsensus is too %s, so we will not "
             "serve it to clients. It was valid %s %s local time and we "
             "continued to serve it for up to 24 hours %s.%s",
             flavor ? flavor : "",
             flavor ? " " : "",
             is_too_new ? "new" : "old",
             is_too_new ? "after" : "until",
             timestamp,
             is_too_new ? "before it was valid" : "after it expired",
             dupes);
    tor_free(dupes);
  }
}

/**
 * Parse a single hex-encoded sha3-256 digest from <b>hex</b> into
 * <b>digest</b>. Return 0 on success.  On failure, report that the hash came
 * from <b>location</b>, report that we are taking <b>action</b> with it, and
 * return -1.
 */
static int
parse_one_diff_hash(uint8_t *digest, const char *hex, const char *location,
                    const char *action)
{
  if (base16_decode((char*)digest, DIGEST256_LEN, hex, strlen(hex)) ==
      DIGEST256_LEN) {
    return 0;
  } else {
    log_fn(LOG_PROTOCOL_WARN, LD_DIR,
           "%s contained bogus digest %s; %s.",
           location, escaped(hex), action);
    return -1;
  }
}

/** If there is an X-Or-Diff-From-Consensus header included in <b>headers</b>,
 * set <b>digest_out</b> to a new smartlist containing every 256-bit
 * hex-encoded digest listed in that header and return 0.  Otherwise return
 * -1.  */
static int
parse_or_diff_from_header(smartlist_t **digests_out, const char *headers)
{
  char *hdr = http_get_header(headers, X_OR_DIFF_FROM_CONSENSUS_HEADER);
  if (hdr == NULL) {
    return -1;
  }
  smartlist_t *hex_digests = smartlist_new();
  *digests_out = smartlist_new();
  smartlist_split_string(hex_digests, hdr, " ",
                         SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, -1);
  SMARTLIST_FOREACH_BEGIN(hex_digests, const char *, hex) {
    uint8_t digest[DIGEST256_LEN];
    if (!parse_one_diff_hash(digest, hex, "X-Or-Diff-From-Consensus header",
                             "ignoring")) {
      smartlist_add(*digests_out, tor_memdup(digest, sizeof(digest)));
    }
  } SMARTLIST_FOREACH_END(hex);
  SMARTLIST_FOREACH(hex_digests, char *, cp, tor_free(cp));
  smartlist_free(hex_digests);
  tor_free(hdr);
  return 0;
}

/** Fallback compression method.  The fallback compression method is used in
 * case a client requests a non-compressed document. We only store compressed
 * documents, so we use this compression method to fetch the document and let
 * the spooling system do the streaming decompression.
 */
#define FALLBACK_COMPRESS_METHOD ZLIB_METHOD

/**
 * Try to find the best consensus diff possible in order to serve a client
 * request for a diff from one of the consensuses in <b>digests</b> to the
 * current consensus of flavor <b>flav</b>.  The client supports the
 * compression methods listed in the <b>compression_methods</b> bitfield:
 * place the method chosen (if any) into <b>compression_used_out</b>.
 */
static struct consensus_cache_entry_t *
find_best_diff(const smartlist_t *digests, int flav,
               unsigned compression_methods,
               compress_method_t *compression_used_out)
{
  struct consensus_cache_entry_t *result = NULL;

  SMARTLIST_FOREACH_BEGIN(digests, const uint8_t *, diff_from) {
    unsigned u;
    for (u = 0; u < ARRAY_LENGTH(srv_meth_pref_precompressed); ++u) {
      compress_method_t method = srv_meth_pref_precompressed[u];
      if (0 == (compression_methods & (1u<<method)))
        continue; // client doesn't like this one, or we don't have it.
      if (consdiffmgr_find_diff_from(&result, flav, DIGEST_SHA3_256,
                                     diff_from, DIGEST256_LEN,
                                     method) == CONSDIFF_AVAILABLE) {
        tor_assert_nonfatal(result);
        *compression_used_out = method;
        return result;
      }
    }
  } SMARTLIST_FOREACH_END(diff_from);

  SMARTLIST_FOREACH_BEGIN(digests, const uint8_t *, diff_from) {
    if (consdiffmgr_find_diff_from(&result, flav, DIGEST_SHA3_256, diff_from,
          DIGEST256_LEN, FALLBACK_COMPRESS_METHOD) == CONSDIFF_AVAILABLE) {
      tor_assert_nonfatal(result);
      *compression_used_out = FALLBACK_COMPRESS_METHOD;
      return result;
    }
  } SMARTLIST_FOREACH_END(diff_from);

  return NULL;
}

/** Lookup the cached consensus document by the flavor found in <b>flav</b>.
 * The preferred set of compression methods should be listed in the
 * <b>compression_methods</b> bitfield. The compression method chosen (if any)
 * is stored in <b>compression_used_out</b>. */
static struct consensus_cache_entry_t *
find_best_consensus(int flav,
                    unsigned compression_methods,
                    compress_method_t *compression_used_out)
{
  struct consensus_cache_entry_t *result = NULL;
  unsigned u;

  for (u = 0; u < ARRAY_LENGTH(srv_meth_pref_precompressed); ++u) {
    compress_method_t method = srv_meth_pref_precompressed[u];

    if (0 == (compression_methods & (1u<<method)))
      continue;

    if (consdiffmgr_find_consensus(&result, flav,
                                   method) == CONSDIFF_AVAILABLE) {
      tor_assert_nonfatal(result);
      *compression_used_out = method;
      return result;
    }
  }

  if (consdiffmgr_find_consensus(&result, flav,
        FALLBACK_COMPRESS_METHOD) == CONSDIFF_AVAILABLE) {
    tor_assert_nonfatal(result);
    *compression_used_out = FALLBACK_COMPRESS_METHOD;
    return result;
  }

  return NULL;
}

/** Try to find the best supported compression method possible from a given
 * <b>compression_methods</b>. Return NO_METHOD if no mutually supported
 * compression method could be found. */
static compress_method_t
find_best_compression_method(unsigned compression_methods, int stream)
{
  unsigned u;
  compress_method_t *methods;
  size_t length;

  if (stream) {
    methods = srv_meth_pref_streaming_compression;
    length = ARRAY_LENGTH(srv_meth_pref_streaming_compression);
  } else {
    methods = srv_meth_pref_precompressed;
    length = ARRAY_LENGTH(srv_meth_pref_precompressed);
  }

  for (u = 0; u < length; ++u) {
    compress_method_t method = methods[u];
    if (compression_methods & (1u<<method))
      return method;
  }

  return NO_METHOD;
}

/** Check if any of the digests in <b>digests</b> matches the latest consensus
 *  flavor (given in <b>flavor</b>) that we have available. */
static int
digest_list_contains_best_consensus(consensus_flavor_t flavor,
                                    const smartlist_t *digests)
{
  const networkstatus_t *ns = NULL;

  if (digests == NULL)
    return 0;

  ns = networkstatus_get_latest_consensus_by_flavor(flavor);

  if (ns == NULL)
    return 0;

  SMARTLIST_FOREACH_BEGIN(digests, const uint8_t *, digest) {
    if (tor_memeq(ns->digest_sha3_as_signed, digest, DIGEST256_LEN))
      return 1;
  } SMARTLIST_FOREACH_END(digest);

  return 0;
}

/** Encodes the results of parsing a consensus request to figure out what
 * consensus, and possibly what diffs, the user asked for. */
typedef struct {
  /** name of the flavor to retrieve. */
  char *flavor;
  /** flavor to retrieve, as enum. */
  consensus_flavor_t flav;
  /** plus-separated list of authority fingerprints; see
   * client_likes_consensus(). Aliases the URL in the request passed to
   * parse_consensus_request(). */
  const char *want_fps;
  /** Optionally, a smartlist of sha3 digests-as-signed of the consensuses
   * to return a diff from. */
  smartlist_t *diff_from_digests;
  /** If true, never send a full consensus. If there is no diff, send
   * a 404 instead. */
  int diff_only;
} parsed_consensus_request_t;

/** Remove all data held in <b>req</b>. Do not free <b>req</b> itself, since
 * it is stack-allocated. */
static void
parsed_consensus_request_clear(parsed_consensus_request_t *req)
{
  if (!req)
    return;
  tor_free(req->flavor);
  if (req->diff_from_digests) {
    SMARTLIST_FOREACH(req->diff_from_digests, uint8_t *, d, tor_free(d));
    smartlist_free(req->diff_from_digests);
  }
  memset(req, 0, sizeof(parsed_consensus_request_t));
}

/**
 * Parse the URL and relevant headers of <b>args</b> for a current-consensus
 * request to learn what flavor of consensus we want, what keys it must be
 * signed with, and what diffs we would accept (or demand) instead. Return 0
 * on success and -1 on failure.
 */
static int
parse_consensus_request(parsed_consensus_request_t *out,
                        const get_handler_args_t *args)
{
  const char *url = args->url;
  memset(out, 0, sizeof(parsed_consensus_request_t));
  out->flav = FLAV_NS;

  const char CONSENSUS_URL_PREFIX[] = "/tor/status-vote/current/consensus/";
  const char CONSENSUS_FLAVORED_PREFIX[] =
    "/tor/status-vote/current/consensus-";

  /* figure out the flavor if any, and who we wanted to sign the thing */
  const char *after_flavor = NULL;

  if (!strcmpstart(url, CONSENSUS_FLAVORED_PREFIX)) {
    const char *f, *cp;
    f = url + strlen(CONSENSUS_FLAVORED_PREFIX);
    cp = strchr(f, '/');
    if (cp) {
      after_flavor = cp+1;
      out->flavor = tor_strndup(f, cp-f);
    } else {
      out->flavor = tor_strdup(f);
    }
    int flav = networkstatus_parse_flavor_name(out->flavor);
    if (flav < 0)
      flav = FLAV_NS;
    out->flav = flav;
  } else {
    if (!strcmpstart(url, CONSENSUS_URL_PREFIX))
      after_flavor = url+strlen(CONSENSUS_URL_PREFIX);
  }

  /* see whether we've been asked explicitly for a diff from an older
   * consensus. (The user might also have said that a diff would be okay,
   * via X-Or-Diff-From-Consensus */
  const char DIFF_COMPONENT[] = "diff/";
  char *diff_hash_in_url = NULL;
  if (after_flavor && !strcmpstart(after_flavor, DIFF_COMPONENT)) {
    after_flavor += strlen(DIFF_COMPONENT);
    const char *cp = strchr(after_flavor, '/');
    if (cp) {
      diff_hash_in_url = tor_strndup(after_flavor, cp-after_flavor);
      out->want_fps = cp+1;
    } else {
      diff_hash_in_url = tor_strdup(after_flavor);
      out->want_fps = NULL;
    }
  } else {
    out->want_fps = after_flavor;
  }

  if (diff_hash_in_url) {
    uint8_t diff_from[DIGEST256_LEN];
    out->diff_from_digests = smartlist_new();
    out->diff_only = 1;
    int ok = !parse_one_diff_hash(diff_from, diff_hash_in_url, "URL",
                                  "rejecting");
    tor_free(diff_hash_in_url);
    if (ok) {
      smartlist_add(out->diff_from_digests,
                    tor_memdup(diff_from, DIGEST256_LEN));
    } else {
      return -1;
    }
  } else {
    parse_or_diff_from_header(&out->diff_from_digests, args->headers);
  }

  return 0;
}

/** Helper function for GET /tor/status-vote/current/consensus
 */
static int
handle_get_current_consensus(dir_connection_t *conn,
                             const get_handler_args_t *args)
{
  const compress_method_t compress_method =
    find_best_compression_method(args->compression_supported, 0);
  const time_t if_modified_since = args->if_modified_since;
  int clear_spool = 0;

  /* v3 network status fetch. */
  long lifetime = NETWORKSTATUS_CACHE_LIFETIME;

  time_t now = time(NULL);
  parsed_consensus_request_t req;

  if (parse_consensus_request(&req, args) < 0) {
    write_short_http_response(conn, 404, "Couldn't parse request");
    goto done;
  }

  if (digest_list_contains_best_consensus(req.flav,
                                          req.diff_from_digests)) {
    write_short_http_response(conn, 304, "Not modified");
    geoip_note_ns_response(GEOIP_REJECT_NOT_MODIFIED);
    goto done;
  }

  struct consensus_cache_entry_t *cached_consensus = NULL;

  compress_method_t compression_used = NO_METHOD;
  if (req.diff_from_digests) {
    cached_consensus = find_best_diff(req.diff_from_digests, req.flav,
                                      args->compression_supported,
                                      &compression_used);
  }

  if (req.diff_only && !cached_consensus) {
    write_short_http_response(conn, 404, "No such diff available");
    geoip_note_ns_response(GEOIP_REJECT_NOT_FOUND);
    goto done;
  }

  if (! cached_consensus) {
    cached_consensus = find_best_consensus(req.flav,
                                           args->compression_supported,
                                           &compression_used);
  }

  time_t valid_after, fresh_until, valid_until;
  int have_valid_after = 0, have_fresh_until = 0, have_valid_until = 0;
  if (cached_consensus) {
    have_valid_after =
      !consensus_cache_entry_get_valid_after(cached_consensus, &valid_after);
    have_fresh_until =
      !consensus_cache_entry_get_fresh_until(cached_consensus, &fresh_until);
    have_valid_until =
      !consensus_cache_entry_get_valid_until(cached_consensus, &valid_until);
  }

  if (cached_consensus && have_valid_after &&
      !networkstatus_valid_after_is_reasonably_live(valid_after, now)) {
    write_short_http_response(conn, 404, "Consensus is too new");
    warn_consensus_is_not_reasonably_live(cached_consensus, req.flavor, now,
                                          1);
    geoip_note_ns_response(GEOIP_REJECT_NOT_FOUND);
    goto done;
  } else if (
      cached_consensus && have_valid_until &&
      !networkstatus_valid_until_is_reasonably_live(valid_until, now)) {
    write_short_http_response(conn, 404, "Consensus is too old");
    warn_consensus_is_not_reasonably_live(cached_consensus, req.flavor, now,
                                          0);
    geoip_note_ns_response(GEOIP_REJECT_NOT_FOUND);
    goto done;
  }

  if (cached_consensus && req.want_fps &&
      !client_likes_consensus(cached_consensus, req.want_fps)) {
    write_short_http_response(conn, 404, "Consensus not signed by sufficient "
                           "number of requested authorities");
    geoip_note_ns_response(GEOIP_REJECT_NOT_ENOUGH_SIGS);
    goto done;
  }

  conn->spool = smartlist_new();
  clear_spool = 1;
  {
    spooled_resource_t *spooled;
    if (cached_consensus) {
      spooled = spooled_resource_new_from_cache_entry(cached_consensus);
      smartlist_add(conn->spool, spooled);
    }
  }

  lifetime = (have_fresh_until && fresh_until > now) ? fresh_until - now : 0;

  size_t size_guess = 0;
  int n_expired = 0;
  dirserv_spool_remove_missing_and_guess_size(conn, if_modified_since,
                                              compress_method != NO_METHOD,
                                              &size_guess,
                                              &n_expired);

  if (!smartlist_len(conn->spool) && !n_expired) {
    write_short_http_response(conn, 404, "Not found");
    geoip_note_ns_response(GEOIP_REJECT_NOT_FOUND);
    goto done;
  } else if (!smartlist_len(conn->spool)) {
    write_short_http_response(conn, 304, "Not modified");
    geoip_note_ns_response(GEOIP_REJECT_NOT_MODIFIED);
    goto done;
  }

  if (connection_dir_is_global_write_low(TO_CONN(conn), size_guess)) {
    log_debug(LD_DIRSERV,
              "Client asked for network status lists, but we've been "
              "writing too many bytes lately. Sending 503 Dir busy.");
    write_short_http_response(conn, 503, "Directory busy, try again later");
    geoip_note_ns_response(GEOIP_REJECT_BUSY);
    goto done;
  }

  tor_addr_t addr;
  if (tor_addr_parse(&addr, (TO_CONN(conn))->address) >= 0) {
    geoip_note_client_seen(GEOIP_CLIENT_NETWORKSTATUS,
                           &addr, NULL,
                           time(NULL));
    geoip_note_ns_response(GEOIP_SUCCESS);
    /* Note that a request for a network status has started, so that we
     * can measure the download time later on. */
    if (conn->dirreq_id)
      geoip_start_dirreq(conn->dirreq_id, size_guess, DIRREQ_TUNNELED);
    else
      geoip_start_dirreq(TO_CONN(conn)->global_identifier, size_guess,
                         DIRREQ_DIRECT);
  }

  /* Use this header to tell caches that the response depends on the
   * X-Or-Diff-From-Consensus header (or lack thereof). */
  const char vary_header[] = "Vary: X-Or-Diff-From-Consensus\r\n";

  clear_spool = 0;

  // The compress_method might have been NO_METHOD, but we store the data
  // compressed. Decompress them using `compression_used`. See fallback code in
  // find_best_consensus() and find_best_diff().
  write_http_response_headers(conn, -1,
                             compress_method == NO_METHOD ?
                               NO_METHOD : compression_used,
                             vary_header,
                             smartlist_len(conn->spool) == 1 ? lifetime : 0);

  if (compress_method == NO_METHOD && smartlist_len(conn->spool))
    conn->compress_state = tor_compress_new(0, compression_used,
                                            HIGH_COMPRESSION);

  /* Prime the connection with some data. */
  const int initial_flush_result = connection_dirserv_flushed_some(conn);
  tor_assert_nonfatal(initial_flush_result == 0);
  goto done;

 done:
  parsed_consensus_request_clear(&req);
  if (clear_spool) {
    dir_conn_clear_spool(conn);
  }
  return 0;
}

/** Helper function for GET /tor/status-vote/{current,next}/...
 */
static int
handle_get_status_vote(dir_connection_t *conn, const get_handler_args_t *args)
{
  const char *url = args->url;
  {
    ssize_t body_len = 0;
    ssize_t estimated_len = 0;
    int lifetime = 60; /* XXXX?? should actually use vote intervals. */
    /* This smartlist holds strings that we can compress on the fly. */
    smartlist_t *items = smartlist_new();
    /* This smartlist holds cached_dir_t objects that have a precompressed
     * deflated version. */
    smartlist_t *dir_items = smartlist_new();
    dirvote_dirreq_get_status_vote(url, items, dir_items);
    if (!smartlist_len(dir_items) && !smartlist_len(items)) {
      write_short_http_response(conn, 404, "Not found");
      goto vote_done;
    }

    /* We're sending items from at most one kind of source */
    tor_assert_nonfatal(smartlist_len(items) == 0 ||
                        smartlist_len(dir_items) == 0);

    int streaming;
    unsigned mask;
    if (smartlist_len(items)) {
      /* We're taking strings and compressing them on the fly. */
      streaming = 1;
      mask = ~0u;
    } else {
      /* We're taking cached_dir_t objects. We only have them uncompressed
       * or deflated. */
      streaming = 0;
      mask = (1u<<NO_METHOD) | (1u<<ZLIB_METHOD);
    }
    const compress_method_t compress_method = find_best_compression_method(
                              args->compression_supported&mask, streaming);

    SMARTLIST_FOREACH(dir_items, cached_dir_t *, d,
                      body_len += compress_method != NO_METHOD ?
                        d->dir_compressed_len : d->dir_len);
    estimated_len += body_len;
    SMARTLIST_FOREACH(items, const char *, item, {
        size_t ln = strlen(item);
        if (compress_method != NO_METHOD) {
          estimated_len += ln/2;
        } else {
          body_len += ln; estimated_len += ln;
        }
      });

    if (connection_dir_is_global_write_low(TO_CONN(conn), estimated_len)) {
      write_short_http_response(conn, 503, "Directory busy, try again later");
      goto vote_done;
    }
    write_http_response_header(conn, body_len ? body_len : -1,
                 compress_method,
                 lifetime);

    if (smartlist_len(items)) {
      if (compress_method != NO_METHOD) {
        conn->compress_state = tor_compress_new(1, compress_method,
                           choose_compression_level());
      }

      SMARTLIST_FOREACH(items, const char *, c,
                        connection_dir_buf_add(c, strlen(c), conn,
                                               c_sl_idx == c_sl_len - 1));
    } else {
      SMARTLIST_FOREACH(dir_items, cached_dir_t *, d,
          connection_buf_add(compress_method != NO_METHOD ?
                                    d->dir_compressed : d->dir,
                                  compress_method != NO_METHOD ?
                                    d->dir_compressed_len : d->dir_len,
                                  TO_CONN(conn)));
    }
  vote_done:
    smartlist_free(items);
    smartlist_free(dir_items);
    goto done;
  }
 done:
  return 0;
}

/** Helper function for GET /tor/micro/d/...
 */
static int
handle_get_microdesc(dir_connection_t *conn, const get_handler_args_t *args)
{
  const char *url = args->url;
  const compress_method_t compress_method =
    find_best_compression_method(args->compression_supported, 1);
  int clear_spool = 1;
  {
    conn->spool = smartlist_new();

    dir_split_resource_into_spoolable(url+strlen("/tor/micro/d/"),
                                      DIR_SPOOL_MICRODESC,
                                      conn->spool, NULL,
                                      DSR_DIGEST256|DSR_BASE64|DSR_SORT_UNIQ);

    size_t size_guess = 0;
    dirserv_spool_remove_missing_and_guess_size(conn, 0,
                                                compress_method != NO_METHOD,
                                                &size_guess, NULL);
    if (smartlist_len(conn->spool) == 0) {
      write_short_http_response(conn, 404, "Not found");
      goto done;
    }
    if (connection_dir_is_global_write_low(TO_CONN(conn), size_guess)) {
      log_info(LD_DIRSERV,
               "Client asked for server descriptors, but we've been "
               "writing too many bytes lately. Sending 503 Dir busy.");
      write_short_http_response(conn, 503, "Directory busy, try again later");
      goto done;
    }

    clear_spool = 0;
    write_http_response_header(conn, -1,
                               compress_method,
                               MICRODESC_CACHE_LIFETIME);

    if (compress_method != NO_METHOD)
      conn->compress_state = tor_compress_new(1, compress_method,
                                      choose_compression_level());

    const int initial_flush_result = connection_dirserv_flushed_some(conn);
    tor_assert_nonfatal(initial_flush_result == 0);
    goto done;
  }

 done:
  if (clear_spool) {
    dir_conn_clear_spool(conn);
  }
  return 0;
}

/** Helper function for GET /tor/{server,extra}/...
 */
static int
handle_get_descriptor(dir_connection_t *conn, const get_handler_args_t *args)
{
  const char *url = args->url;
  const compress_method_t compress_method =
    find_best_compression_method(args->compression_supported, 1);
  const or_options_t *options = get_options();
  int clear_spool = 1;
  if (!strcmpstart(url,"/tor/server/") ||
      (!options->BridgeAuthoritativeDir &&
       !options->BridgeRelay && !strcmpstart(url,"/tor/extra/"))) {
    int res;
    const char *msg = NULL;
    int cache_lifetime = 0;
    int is_extra = !strcmpstart(url,"/tor/extra/");
    url += is_extra ? strlen("/tor/extra/") : strlen("/tor/server/");
    dir_spool_source_t source;
    time_t publish_cutoff = 0;
    if (!strcmpstart(url, "d/")) {
      source =
        is_extra ? DIR_SPOOL_EXTRA_BY_DIGEST : DIR_SPOOL_SERVER_BY_DIGEST;
    } else {
      source =
        is_extra ? DIR_SPOOL_EXTRA_BY_FP : DIR_SPOOL_SERVER_BY_FP;
      /* We only want to apply a publish cutoff when we're requesting
       * resources by fingerprint. */
      publish_cutoff = time(NULL) - ROUTER_MAX_AGE_TO_PUBLISH;
    }

    conn->spool = smartlist_new();
    res = dirserv_get_routerdesc_spool(conn->spool, url,
                                       source,
                                       connection_dir_is_encrypted(conn),
                                       &msg);

    if (!strcmpstart(url, "all")) {
      cache_lifetime = FULL_DIR_CACHE_LIFETIME;
    } else if (smartlist_len(conn->spool) == 1) {
      cache_lifetime = ROUTERDESC_BY_DIGEST_CACHE_LIFETIME;
    }

    size_t size_guess = 0;
    int n_expired = 0;
    dirserv_spool_remove_missing_and_guess_size(conn, publish_cutoff,
                                                compress_method != NO_METHOD,
                                                &size_guess, &n_expired);

    /* If we are the bridge authority and the descriptor is a bridge
     * descriptor, remember that we served this descriptor for desc stats. */
    /* XXXX it's a bit of a kludge to have this here. */
    if (get_options()->BridgeAuthoritativeDir &&
        source == DIR_SPOOL_SERVER_BY_FP) {
      SMARTLIST_FOREACH_BEGIN(conn->spool, spooled_resource_t *, spooled) {
        const routerinfo_t *router =
          router_get_by_id_digest((const char *)spooled->digest);
        /* router can be NULL here when the bridge auth is asked for its own
         * descriptor. */
        if (router && router->purpose == ROUTER_PURPOSE_BRIDGE)
          rep_hist_note_desc_served(router->cache_info.identity_digest);
      } SMARTLIST_FOREACH_END(spooled);
    }

    if (res < 0 || size_guess == 0 || smartlist_len(conn->spool) == 0) {
      if (msg == NULL)
        msg = "Not found";
      write_short_http_response(conn, 404, msg);
    } else {
      if (connection_dir_is_global_write_low(TO_CONN(conn), size_guess)) {
        log_info(LD_DIRSERV,
                 "Client asked for server descriptors, but we've been "
                 "writing too many bytes lately. Sending 503 Dir busy.");
        write_short_http_response(conn, 503,
                                  "Directory busy, try again later");
        dir_conn_clear_spool(conn);
        goto done;
      }
      write_http_response_header(conn, -1, compress_method, cache_lifetime);
      if (compress_method != NO_METHOD)
        conn->compress_state = tor_compress_new(1, compress_method,
                                        choose_compression_level());
      clear_spool = 0;
      /* Prime the connection with some data. */
      int initial_flush_result = connection_dirserv_flushed_some(conn);
      tor_assert_nonfatal(initial_flush_result == 0);
    }
    goto done;
  }
 done:
  if (clear_spool)
    dir_conn_clear_spool(conn);
  return 0;
}

/** Helper function for GET /tor/keys/...
 */
static int
handle_get_keys(dir_connection_t *conn, const get_handler_args_t *args)
{
  const char *url = args->url;
  const compress_method_t compress_method =
    find_best_compression_method(args->compression_supported, 1);
  const time_t if_modified_since = args->if_modified_since;
  {
    smartlist_t *certs = smartlist_new();
    ssize_t len = -1;
    if (!strcmp(url, "/tor/keys/all")) {
      authority_cert_get_all(certs);
    } else if (!strcmp(url, "/tor/keys/authority")) {
      authority_cert_t *cert = get_my_v3_authority_cert();
      if (cert)
        smartlist_add(certs, cert);
    } else if (!strcmpstart(url, "/tor/keys/fp/")) {
      smartlist_t *fps = smartlist_new();
      dir_split_resource_into_fingerprints(url+strlen("/tor/keys/fp/"),
                                           fps, NULL,
                                           DSR_HEX|DSR_SORT_UNIQ);
      SMARTLIST_FOREACH(fps, char *, d, {
          authority_cert_t *c = authority_cert_get_newest_by_id(d);
          if (c) smartlist_add(certs, c);
          tor_free(d);
      });
      smartlist_free(fps);
    } else if (!strcmpstart(url, "/tor/keys/sk/")) {
      smartlist_t *fps = smartlist_new();
      dir_split_resource_into_fingerprints(url+strlen("/tor/keys/sk/"),
                                           fps, NULL,
                                           DSR_HEX|DSR_SORT_UNIQ);
      SMARTLIST_FOREACH(fps, char *, d, {
          authority_cert_t *c = authority_cert_get_by_sk_digest(d);
          if (c) smartlist_add(certs, c);
          tor_free(d);
      });
      smartlist_free(fps);
    } else if (!strcmpstart(url, "/tor/keys/fp-sk/")) {
      smartlist_t *fp_sks = smartlist_new();
      dir_split_resource_into_fingerprint_pairs(url+strlen("/tor/keys/fp-sk/"),
                                                fp_sks);
      SMARTLIST_FOREACH(fp_sks, fp_pair_t *, pair, {
          authority_cert_t *c = authority_cert_get_by_digests(pair->first,
                                                              pair->second);
          if (c) smartlist_add(certs, c);
          tor_free(pair);
      });
      smartlist_free(fp_sks);
    } else {
      write_short_http_response(conn, 400, "Bad request");
      goto keys_done;
    }
    if (!smartlist_len(certs)) {
      write_short_http_response(conn, 404, "Not found");
      goto keys_done;
    }
    SMARTLIST_FOREACH(certs, authority_cert_t *, c,
      if (c->cache_info.published_on < if_modified_since)
        SMARTLIST_DEL_CURRENT(certs, c));
    if (!smartlist_len(certs)) {
      write_short_http_response(conn, 304, "Not modified");
      goto keys_done;
    }
    len = 0;
    SMARTLIST_FOREACH(certs, authority_cert_t *, c,
                      len += c->cache_info.signed_descriptor_len);

    if (connection_dir_is_global_write_low(TO_CONN(conn),
                                compress_method != NO_METHOD ? len/2 : len)) {
      write_short_http_response(conn, 503, "Directory busy, try again later");
      goto keys_done;
    }

    write_http_response_header(conn,
                               compress_method != NO_METHOD ? -1 : len,
                               compress_method,
                               60*60);
    if (compress_method != NO_METHOD) {
      conn->compress_state = tor_compress_new(1, compress_method,
                                              choose_compression_level());
    }

    SMARTLIST_FOREACH(certs, authority_cert_t *, c,
          connection_dir_buf_add(c->cache_info.signed_descriptor_body,
                                 c->cache_info.signed_descriptor_len,
                                 conn, c_sl_idx == c_sl_len - 1));
 keys_done:
    smartlist_free(certs);
    goto done;
  }
 done:
  return 0;
}

/** Helper function for GET `/tor/hs/3/...`. Only for version 3.
 */
STATIC int
handle_get_hs_descriptor_v3(dir_connection_t *conn,
                            const get_handler_args_t *args)
{
  int retval;
  const char *desc_str = NULL;
  const char *pubkey_str = NULL;
  const char *url = args->url;

  /* Reject non anonymous dir connections (which also tests if encrypted). We
   * do not allow single hop clients to query an HSDir. */
  if (!connection_dir_is_anonymous(conn)) {
    write_short_http_response(conn, 503,
                              "Rejecting single hop HS v3 descriptor request");
    goto done;
  }

  /* After the path prefix follows the base64 encoded blinded pubkey which we
   * use to get the descriptor from the cache. Skip the prefix and get the
   * pubkey. */
  tor_assert(!strcmpstart(url, "/tor/hs/3/"));
  pubkey_str = url + strlen("/tor/hs/3/");
  retval = hs_cache_lookup_as_dir(HS_VERSION_THREE,
                                  pubkey_str, &desc_str);
  if (retval <= 0 || desc_str == NULL) {
    write_short_http_response(conn, 404, "Not found");
    goto done;
  }

  /* Found requested descriptor! Pass it to this nice client. */
  write_http_response_header(conn, strlen(desc_str), NO_METHOD, 0);
  connection_buf_add(desc_str, strlen(desc_str), TO_CONN(conn));

 done:
  return 0;
}

/** Helper function for GET /tor/networkstatus-bridges
 */
static int
handle_get_networkstatus_bridges(dir_connection_t *conn,
                                 const get_handler_args_t *args)
{
  const char *headers = args->headers;

  const or_options_t *options = get_options();
  if (options->BridgeAuthoritativeDir &&
      options->BridgePassword_AuthDigest_ &&
      connection_dir_is_encrypted(conn)) {
    char *status;
    char digest[DIGEST256_LEN];

    char *header = http_get_header(headers, "Authorization: Basic ");
    if (header)
      crypto_digest256(digest, header, strlen(header), DIGEST_SHA256);

    /* now make sure the password is there and right */
    if (!header ||
        tor_memneq(digest,
                   options->BridgePassword_AuthDigest_, DIGEST256_LEN)) {
      write_short_http_response(conn, 404, "Not found");
      tor_free(header);
      goto done;
    }
    tor_free(header);

    /* all happy now. send an answer. */
    status = networkstatus_getinfo_by_purpose("bridge", time(NULL));
    size_t dlen = strlen(status);
    write_http_response_header(conn, dlen, NO_METHOD, 0);
    connection_buf_add(status, dlen, TO_CONN(conn));
    tor_free(status);
    goto done;
  }
 done:
  return 0;
}

/** Helper function for GET the bandwidth file used for the next vote */
static int
handle_get_next_bandwidth(dir_connection_t *conn,
                          const get_handler_args_t *args)
{
  log_debug(LD_DIR, "Getting next bandwidth.");
  const or_options_t *options = get_options();
  const compress_method_t compress_method =
    find_best_compression_method(args->compression_supported, 1);

  if (options->V3BandwidthsFile) {
    char *bandwidth = read_file_to_str(options->V3BandwidthsFile,
                                       RFTS_IGNORE_MISSING, NULL);
    if (bandwidth != NULL) {
      ssize_t len = strlen(bandwidth);
      write_http_response_header(conn, compress_method != NO_METHOD ? -1 : len,
                                 compress_method, BANDWIDTH_CACHE_LIFETIME);
      if (compress_method != NO_METHOD) {
        conn->compress_state = tor_compress_new(1, compress_method,
                                        choose_compression_level());
        log_debug(LD_DIR, "Compressing bandwidth file.");
      } else {
        log_debug(LD_DIR, "Not compressing bandwidth file.");
      }
      connection_dir_buf_add((const char*)bandwidth, len, conn, 1);
      tor_free(bandwidth);
      return 0;
    }
  }
  write_short_http_response(conn, 404, "Not found");
  return 0;
}

/** Helper function for GET robots.txt or /tor/robots.txt */
static int
handle_get_robots(dir_connection_t *conn, const get_handler_args_t *args)
{
  (void)args;
  {
    const char robots[] = "User-agent: *\r\nDisallow: /\r\n";
    size_t len = strlen(robots);
    write_http_response_header(conn, len, NO_METHOD, ROBOTS_CACHE_LIFETIME);
    connection_buf_add(robots, len, TO_CONN(conn));
  }
  return 0;
}

/* Given the <b>url</b> from a POST request, try to extract the version number
 * using the provided <b>prefix</b>. The version should be after the prefix and
 * ending with the separator "/". For instance:
 *      /tor/hs/3/publish
 *
 * On success, <b>end_pos</b> points to the position right after the version
 * was found. On error, it is set to NULL.
 *
 * Return version on success else negative value. */
STATIC int
parse_hs_version_from_post(const char *url, const char *prefix,
                           const char **end_pos)
{
  int ok;
  unsigned long version;
  const char *start;
  char *end = NULL;

  tor_assert(url);
  tor_assert(prefix);
  tor_assert(end_pos);

  /* Check if the prefix does start the url. */
  if (strcmpstart(url, prefix)) {
    goto err;
  }
  /* Move pointer to the end of the prefix string. */
  start = url + strlen(prefix);
  /* Try this to be the HS version and if we are still at the separator, next
   * will be move to the right value. */
  version = tor_parse_long(start, 10, 0, INT_MAX, &ok, &end);
  if (!ok) {
    goto err;
  }

  *end_pos = end;
  return (int) version;
 err:
  *end_pos = NULL;
  return -1;
}

/* Handle the POST request for a hidden service descripror. The request is in
 * <b>url</b>, the body of the request is in <b>body</b>. Return 200 on success
 * else return 400 indicating a bad request. */
STATIC int
handle_post_hs_descriptor(const char *url, const char *body)
{
  int version;
  const char *end_pos;

  tor_assert(url);
  tor_assert(body);

  version = parse_hs_version_from_post(url, "/tor/hs/", &end_pos);
  if (version < 0) {
    goto err;
  }

  /* We have a valid version number, now make sure it's a publish request. Use
   * the end position just after the version and check for the command. */
  if (strcmpstart(end_pos, "/publish")) {
    goto err;
  }

  switch (version) {
  case HS_VERSION_THREE:
    if (hs_cache_store_as_dir(body) < 0) {
      goto err;
    }
    log_info(LD_REND, "Publish request for HS descriptor handled "
                      "successfully.");
    break;
  default:
    /* Unsupported version, return a bad request. */
    goto err;
  }

  return 200;
 err:
  /* Bad request. */
  return 400;
}

/** Helper function: called when a dirserver gets a complete HTTP POST
 * request.  Look for an uploaded server descriptor or rendezvous
 * service descriptor.  On finding one, process it and write a
 * response into conn-\>outbuf.  If the request is unrecognized, send a
 * 400.  Always return 0. */
MOCK_IMPL(STATIC int,
directory_handle_command_post,(dir_connection_t *conn, const char *headers,
                               const char *body, size_t body_len))
{
  char *url = NULL;
  const or_options_t *options = get_options();

  (void) body_len;

  log_debug(LD_DIRSERV,"Received POST command.");

  conn->base_.state = DIR_CONN_STATE_SERVER_WRITING;

  if (!public_server_mode(options)) {
    log_info(LD_DIR, "Rejected dir post request from %s "
             "since we're not a public relay.",
             connection_describe_peer(TO_CONN(conn)));
    write_short_http_response(conn, 503, "Not acting as a public relay");
    goto done;
  }

  if (parse_http_url(headers, &url) < 0) {
    write_short_http_response(conn, 400, "Bad request");
    return 0;
  }
  log_debug(LD_DIRSERV,"rewritten url as '%s'.", escaped(url));

  /* Handle HS descriptor publish request. We force an anonymous connection
   * (which also tests for encrypted). We do not allow single-hop client to
   * post a descriptor onto an HSDir. */
  if (!strcmpstart(url, "/tor/hs/")) {
    if (!connection_dir_is_anonymous(conn)) {
      write_short_http_response(conn, 503,
                                "Rejecting single hop HS descriptor post");
      goto done;
    }
    const char *msg = "HS descriptor stored successfully.";

    /* We most probably have a publish request for an HS descriptor. */
    int code = handle_post_hs_descriptor(url, body);
    if (code != 200) {
      msg = "Invalid HS descriptor. Rejected.";
    }
    write_short_http_response(conn, code, msg);
    goto done;
  }

  if (!authdir_mode(options)) {
    /* we just provide cached directories; we don't want to
     * receive anything. */
    write_short_http_response(conn, 400, "Nonauthoritative directory does not "
                           "accept posted server descriptors");
    goto done;
  }

  if (authdir_mode(options) &&
      !strcmp(url,"/tor/")) { /* server descriptor post */
    const char *msg = "[None]";
    uint8_t purpose = authdir_mode_bridge(options) ?
                      ROUTER_PURPOSE_BRIDGE : ROUTER_PURPOSE_GENERAL;

    {
      char *genreason = http_get_header(headers, "X-Desc-Gen-Reason: ");
      log_info(LD_DIRSERV,
               "New descriptor post, because: %s",
               genreason ? genreason : "not specified");
      tor_free(genreason);
    }

    was_router_added_t r = dirserv_add_multiple_descriptors(body, body_len,
                                           purpose, conn->base_.address, &msg);
    tor_assert(msg);

    if (r == ROUTER_ADDED_SUCCESSFULLY) {
      write_short_http_response(conn, 200, msg);
    } else if (WRA_WAS_OUTDATED(r)) {
      write_http_response_header_impl(conn, -1, NULL, NULL,
                                      "X-Descriptor-Not-New: Yes\r\n", -1);
    } else {
      log_info(LD_DIRSERV,
               "Rejected router descriptor or extra-info from %s "
               "(\"%s\").",
               connection_describe_peer(TO_CONN(conn)),
               msg);
      write_short_http_response(conn, 400, msg);
    }
    goto done;
  }

  if (authdir_mode_v3(options) &&
      !strcmp(url,"/tor/post/vote")) { /* v3 networkstatus vote */
    const char *msg = "OK";
    int status;
    if (dirvote_add_vote(body, approx_time(), TO_CONN(conn)->address,
                         &msg, &status)) {
      write_short_http_response(conn, status, "Vote stored");
    } else {
      tor_assert(msg);
      log_warn(LD_DIRSERV, "Rejected vote from %s (\"%s\").",
               connection_describe_peer(TO_CONN(conn)),
               msg);
      write_short_http_response(conn, status, msg);
    }
    goto done;
  }

  if (authdir_mode_v3(options) &&
      !strcmp(url,"/tor/post/consensus-signature")) { /* sigs on consensus. */
    const char *msg = NULL;
    if (dirvote_add_signatures(body, conn->base_.address, &msg)>=0) {
      write_short_http_response(conn, 200, msg?msg:"Signatures stored");
    } else {
      log_warn(LD_DIR, "Unable to store signatures posted by %s: %s",
               connection_describe_peer(TO_CONN(conn)),
               msg?msg:"???");
      write_short_http_response(conn, 400,
                                msg?msg:"Unable to store signatures");
    }
    goto done;
  }

  /* we didn't recognize the url */
  write_short_http_response(conn, 404, "Not found");

 done:
  tor_free(url);
  return 0;
}

/** If <b>headers</b> indicates that a proxy was involved, then rewrite
 * <b>conn</b>-\>address to describe our best guess of the address that
 * originated this HTTP request. */
static void
http_set_address_origin(const char *headers, connection_t *conn)
{
  char *fwd;

  fwd = http_get_header(headers, "Forwarded-For: ");
  if (!fwd)
    fwd = http_get_header(headers, "X-Forwarded-For: ");
  if (fwd) {
    tor_addr_t toraddr;
    if (tor_addr_parse(&toraddr,fwd) == -1 ||
        tor_addr_is_internal(&toraddr,0)) {
      log_debug(LD_DIR, "Ignoring local/internal IP %s", escaped(fwd));
      tor_free(fwd);
      return;
    }

    tor_free(conn->address);
    conn->address = tor_strdup(fwd);
    tor_free(fwd);
  }
}

/** Called when a dirserver receives data on a directory connection;
 * looks for an HTTP request.  If the request is complete, remove it
 * from the inbuf, try to process it; otherwise, leave it on the
 * buffer.  Return a 0 on success, or -1 on error.
 */
int
directory_handle_command(dir_connection_t *conn)
{
  char *headers=NULL, *body=NULL;
  size_t body_len=0;
  int r;

  tor_assert(conn);
  tor_assert(conn->base_.type == CONN_TYPE_DIR);

  switch (connection_fetch_from_buf_http(TO_CONN(conn),
                              &headers, MAX_HEADERS_SIZE,
                              &body, &body_len, MAX_DIR_UL_SIZE, 0)) {
    case -1: /* overflow */
      log_warn(LD_DIRSERV,
               "Request too large from %s to DirPort. Closing.",
               connection_describe_peer(TO_CONN(conn)));
      return -1;
    case 0:
      log_debug(LD_DIRSERV,"command not all here yet.");
      return 0;
    /* case 1, fall through */
  }

  http_set_address_origin(headers, TO_CONN(conn));
  // we should escape headers here as well,
  // but we can't call escaped() twice, as it uses the same buffer
  //log_debug(LD_DIRSERV,"headers %s, body %s.", headers, escaped(body));

  if (!strncasecmp(headers,"GET",3))
    r = directory_handle_command_get(conn, headers, body, body_len);
  else if (!strncasecmp(headers,"POST",4))
    r = directory_handle_command_post(conn, headers, body, body_len);
  else {
    log_fn(LOG_PROTOCOL_WARN, LD_PROTOCOL,
           "Got headers %s with unknown command. Closing.",
           escaped(headers));
    r = -1;
  }

  tor_free(headers); tor_free(body);
  return r;
}
