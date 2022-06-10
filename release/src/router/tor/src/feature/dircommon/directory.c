/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "core/or/or.h"

#include "app/config/config.h"
#include "core/mainloop/connection.h"
#include "core/or/circuitlist.h"
#include "core/or/connection_edge.h"
#include "core/or/connection_or.h"
#include "core/or/channeltls.h"
#include "feature/dircache/dircache.h"
#include "feature/dircache/dirserv.h"
#include "feature/dirclient/dirclient.h"
#include "feature/dircommon/directory.h"
#include "feature/dircommon/fp_pair.h"
#include "feature/stats/geoip_stats.h"
#include "lib/compress/compress.h"

#include "core/or/circuit_st.h"
#include "core/or/or_circuit_st.h"
#include "core/or/edge_connection_st.h"
#include "core/or/or_connection_st.h"
#include "feature/dircommon/dir_connection_st.h"
#include "feature/nodelist/routerinfo_st.h"

/**
 * \file directory.c
 * \brief Code to send and fetch information from directory authorities and
 * caches via HTTP.
 *
 * Directory caches and authorities use dirserv.c to generate the results of a
 * query and stream them to the connection; clients use routerparse.c to parse
 * them.
 *
 * Every directory request has a dir_connection_t on the client side and on
 * the server side.  In most cases, the dir_connection_t object is a linked
 * connection, tunneled through an edge_connection_t so that it can be a
 * stream on the Tor network.  The only non-tunneled connections are those
 * that are used to upload material (descriptors and votes) to authorities.
 * Among tunneled connections, some use one-hop circuits, and others use
 * multi-hop circuits for anonymity.
 *
 * Directory requests are launched by calling
 * directory_initiate_request(). This
 * launch the connection, will construct an HTTP request with
 * directory_send_command(), send the and wait for a response.  The client
 * later handles the response with connection_dir_client_reached_eof(),
 * which passes the information received to another part of Tor.
 *
 * On the server side, requests are read in directory_handle_command(),
 * which dispatches first on the request type (GET or POST), and then on
 * the URL requested. GET requests are processed with a table-based
 * dispatcher in url_table[].  The process of handling larger GET requests
 * is complicated because we need to avoid allocating a copy of all the
 * data to be sent to the client in one huge buffer.  Instead, we spool the
 * data into the buffer using logic in connection_dirserv_flushed_some() in
 * dirserv.c.  (TODO: If we extended buf.c to have a zero-copy
 * reference-based buffer type, we could remove most of that code, at the
 * cost of a bit more reference counting.)
 **/

/* In-points to directory.c:
 *
 * - directory_post_to_dirservers(), called from
 *   router_upload_dir_desc_to_dirservers() in router.c
 *   upload_service_descriptor() in rendservice.c
 * - directory_get_from_dirserver(), called from
 *   run_scheduled_events() in main.c
 *   do_hup() in main.c
 * - connection_dir_process_inbuf(), called from
 *   connection_process_inbuf() in connection.c
 * - connection_dir_finished_flushing(), called from
 *   connection_finished_flushing() in connection.c
 * - connection_dir_finished_connecting(), called from
 *   connection_finished_connecting() in connection.c
 */

/**
 * Cast a `connection_t *` to a `dir_connection_t *`.
 *
 * Exit with an assertion failure if the input is not a
 * `dir_connection_t`.
 **/
dir_connection_t *
TO_DIR_CONN(connection_t *c)
{
  tor_assert(c->magic == DIR_CONNECTION_MAGIC);
  return DOWNCAST(dir_connection_t, c);
}

/**
 * Cast a `const connection_t *` to a `const dir_connection_t *`.
 *
 * Exit with an assertion failure if the input is not a
 * `dir_connection_t`.
 **/
const dir_connection_t *
CONST_TO_DIR_CONN(const connection_t *c)
{
  return TO_DIR_CONN((connection_t *)c);
}

/** Return false if the directory purpose <b>dir_purpose</b>
 * does not require an anonymous (three-hop) connection.
 *
 * Return true 1) by default, 2) if all directory actions have
 * specifically been configured to be over an anonymous connection,
 * or 3) if the router is a bridge */
int
purpose_needs_anonymity(uint8_t dir_purpose, uint8_t router_purpose,
                        const char *resource)
{
  if (get_options()->AllDirActionsPrivate)
    return 1;

  if (router_purpose == ROUTER_PURPOSE_BRIDGE) {
    if (dir_purpose == DIR_PURPOSE_FETCH_SERVERDESC
        && resource && !strcmp(resource, "authority.z")) {
      /* We are asking a bridge for its own descriptor. That doesn't need
         anonymity. */
      return 0;
    }
    /* Assume all other bridge stuff needs anonymity. */
    return 1; /* if no circuits yet, this might break bootstrapping, but it's
               * needed to be safe. */
  }

  switch (dir_purpose)
  {
    case DIR_PURPOSE_UPLOAD_DIR:
    case DIR_PURPOSE_UPLOAD_VOTE:
    case DIR_PURPOSE_UPLOAD_SIGNATURES:
    case DIR_PURPOSE_FETCH_STATUS_VOTE:
    case DIR_PURPOSE_FETCH_DETACHED_SIGNATURES:
    case DIR_PURPOSE_FETCH_CONSENSUS:
    case DIR_PURPOSE_FETCH_CERTIFICATE:
    case DIR_PURPOSE_FETCH_SERVERDESC:
    case DIR_PURPOSE_FETCH_EXTRAINFO:
    case DIR_PURPOSE_FETCH_MICRODESC:
      return 0;
    case DIR_PURPOSE_HAS_FETCHED_HSDESC:
    case DIR_PURPOSE_FETCH_HSDESC:
    case DIR_PURPOSE_UPLOAD_HSDESC:
      return 1;
    case DIR_PURPOSE_SERVER:
    default:
      log_warn(LD_BUG, "Called with dir_purpose=%d, router_purpose=%d",
               dir_purpose, router_purpose);
      tor_assert_nonfatal_unreached();
      return 1; /* Assume it needs anonymity; better safe than sorry. */
  }
}

/** Return a newly allocated string describing <b>auth</b>. Only describes
 * authority features. */
char *
authdir_type_to_string(dirinfo_type_t auth)
{
  char *result;
  smartlist_t *lst = smartlist_new();
  if (auth & V3_DIRINFO)
    smartlist_add(lst, (void*)"V3");
  if (auth & BRIDGE_DIRINFO)
    smartlist_add(lst, (void*)"Bridge");
  if (smartlist_len(lst)) {
    result = smartlist_join_strings(lst, ", ", 0, NULL);
  } else {
    result = tor_strdup("[Not an authority]");
  }
  smartlist_free(lst);
  return result;
}

/** Return true iff anything we say on <b>conn</b> is being encrypted before
 * we send it to the client/server. */
int
connection_dir_is_encrypted(const dir_connection_t *conn)
{
  /* Right now it's sufficient to see if conn is or has been linked, since
   * the only thing it could be linked to is an edge connection on a
   * circuit, and the only way it could have been unlinked is at the edge
   * connection getting closed.
   */
  return TO_CONN(conn)->linked;
}

/** Return true iff the given directory connection <b>dir_conn</b> is
 * anonymous, that is, it is on a circuit via a public relay and not directly
 * from a client or bridge.
 *
 * For client circuits via relays: true for 2-hop+ paths.
 * For client circuits via bridges: true for 3-hop+ paths.
 *
 * This first test if the connection is encrypted since it is a strong
 * requirement for anonymity. */
bool
connection_dir_is_anonymous(const dir_connection_t *dir_conn)
{
  const connection_t *conn, *linked_conn;
  const edge_connection_t *edge_conn;
  const circuit_t *circ;

  tor_assert(dir_conn);

  if (!connection_dir_is_encrypted(dir_conn)) {
    return false;
  }

  /*
   * Buckle up, we'll do a deep dive into the connection in order to get the
   * final connection channel of that connection in order to figure out if
   * this is a client or relay link.
   *
   * We go: dir_conn -> linked_conn -> edge_conn -> on_circuit -> p_chan.
   */

  conn = TO_CONN(dir_conn);
  linked_conn = conn->linked_conn;

  /* The dir connection should be connected to an edge connection. It can not
   * be closed or marked for close. */
  if (linked_conn == NULL || linked_conn->magic != EDGE_CONNECTION_MAGIC ||
      conn->linked_conn_is_closed || conn->linked_conn->marked_for_close) {
    log_debug(LD_DIR, "Directory connection is not anonymous: "
                      "not linked to edge");
    return false;
  }

  edge_conn = CONST_TO_EDGE_CONN(linked_conn);
  circ = edge_conn->on_circuit;

  /* Can't be a circuit we initiated and without a circuit, no channel. */
  if (circ == NULL || CIRCUIT_IS_ORIGIN(circ)) {
    log_debug(LD_DIR, "Directory connection is not anonymous: "
                      "not on OR circuit");
    return false;
  }

  /* It is possible that the circuit was closed because one of the channel was
   * closed or a DESTROY cell was received. Either way, this connection can
   * not continue so return that it is not anonymous since we can not know for
   * sure if it is. */
  if (circ->marked_for_close) {
    log_debug(LD_DIR, "Directory connection is not anonymous: "
                      "circuit marked for close");
    return false;
  }

  /* Get the previous channel to learn if it is a client or relay link. We
   * BUG() because if the circuit is not mark for close, we ought to have a
   * p_chan else we have a code flow issue. */
  if (BUG(CONST_TO_OR_CIRCUIT(circ)->p_chan == NULL)) {
    log_debug(LD_DIR, "Directory connection is not anonymous: "
                      "no p_chan on circuit");
    return false;
  }

  /* Will be true if the channel is an unauthenticated peer which is only true
   * for clients and bridges. */
  return !channel_is_client(CONST_TO_OR_CIRCUIT(circ)->p_chan);
}

/** Parse an HTTP request line at the start of a headers string.  On failure,
 * return -1.  On success, set *<b>command_out</b> to a copy of the HTTP
 * command ("get", "post", etc), set *<b>url_out</b> to a copy of the URL, and
 * return 0. */
int
parse_http_command(const char *headers, char **command_out, char **url_out)
{
  const char *command, *end_of_command;
  char *s, *start, *tmp;

  s = (char *)eat_whitespace_no_nl(headers);
  if (!*s) return -1;
  command = s;
  s = (char *)find_whitespace(s); /* get past GET/POST */
  if (!*s) return -1;
  end_of_command = s;
  s = (char *)eat_whitespace_no_nl(s);
  if (!*s) return -1;
  start = s; /* this is the URL, assuming it's valid */
  s = (char *)find_whitespace(start);
  if (!*s) return -1;

  /* tolerate the http[s] proxy style of putting the hostname in the url */
  if (s-start >= 4 && !strcmpstart(start,"http")) {
    tmp = start + 4;
    if (*tmp == 's')
      tmp++;
    if (s-tmp >= 3 && !strcmpstart(tmp,"://")) {
      tmp = strchr(tmp+3, '/');
      if (tmp && tmp < s) {
        log_debug(LD_DIR,"Skipping over 'http[s]://hostname/' string");
        start = tmp;
      }
    }
  }

  /* Check if the header is well formed (next sequence
   * should be HTTP/1.X\r\n). Assumes we're supporting 1.0? */
  {
    unsigned minor_ver;
    char ch;
    char *e = (char *)eat_whitespace_no_nl(s);
    if (2 != tor_sscanf(e, "HTTP/1.%u%c", &minor_ver, &ch)) {
      return -1;
    }
    if (ch != '\r')
      return -1;
  }

  *url_out = tor_memdup_nulterm(start, s-start);
  *command_out = tor_memdup_nulterm(command, end_of_command - command);
  return 0;
}

/** Return a copy of the first HTTP header in <b>headers</b> whose key is
 * <b>which</b>.  The key should be given with a terminating colon and space;
 * this function copies everything after, up to but not including the
 * following \\r\\n. */
char *
http_get_header(const char *headers, const char *which)
{
  const char *cp = headers;
  while (cp) {
    if (!strcasecmpstart(cp, which)) {
      char *eos;
      cp += strlen(which);
      if ((eos = strchr(cp,'\r')))
        return tor_strndup(cp, eos-cp);
      else
        return tor_strdup(cp);
    }
    cp = strchr(cp, '\n');
    if (cp)
      ++cp;
  }
  return NULL;
}
/** Parse an HTTP response string <b>headers</b> of the form
 * \verbatim
 * "HTTP/1.\%d \%d\%s\r\n...".
 * \endverbatim
 *
 * If it's well-formed, assign the status code to *<b>code</b> and
 * return 0.  Otherwise, return -1.
 *
 * On success: If <b>date</b> is provided, set *date to the Date
 * header in the http headers, or 0 if no such header is found.  If
 * <b>compression</b> is provided, set *<b>compression</b> to the
 * compression method given in the Content-Encoding header, or 0 if no
 * such header is found, or -1 if the value of the header is not
 * recognized.  If <b>reason</b> is provided, strdup the reason string
 * into it.
 */
int
parse_http_response(const char *headers, int *code, time_t *date,
                    compress_method_t *compression, char **reason)
{
  unsigned n1, n2;
  char datestr[RFC1123_TIME_LEN+1];
  smartlist_t *parsed_headers;
  tor_assert(headers);
  tor_assert(code);

  while (TOR_ISSPACE(*headers)) headers++; /* tolerate leading whitespace */

  if (tor_sscanf(headers, "HTTP/1.%u %u", &n1, &n2) < 2 ||
      (n1 != 0 && n1 != 1) ||
      (n2 < 100 || n2 >= 600)) {
    log_warn(LD_HTTP,"Failed to parse header %s",escaped(headers));
    return -1;
  }
  *code = n2;

  parsed_headers = smartlist_new();
  smartlist_split_string(parsed_headers, headers, "\n",
                         SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, -1);
  if (reason) {
    smartlist_t *status_line_elements = smartlist_new();
    tor_assert(smartlist_len(parsed_headers));
    smartlist_split_string(status_line_elements,
                           smartlist_get(parsed_headers, 0),
                           " ", SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, 3);
    tor_assert(smartlist_len(status_line_elements) <= 3);
    if (smartlist_len(status_line_elements) == 3) {
      *reason = smartlist_get(status_line_elements, 2);
      smartlist_set(status_line_elements, 2, NULL); /* Prevent free */
    }
    SMARTLIST_FOREACH(status_line_elements, char *, cp, tor_free(cp));
    smartlist_free(status_line_elements);
  }
  if (date) {
    *date = 0;
    SMARTLIST_FOREACH(parsed_headers, const char *, s,
      if (!strcmpstart(s, "Date: ")) {
        strlcpy(datestr, s+6, sizeof(datestr));
        /* This will do nothing on failure, so we don't need to check
           the result.   We shouldn't warn, since there are many other valid
           date formats besides the one we use. */
        parse_rfc1123_time(datestr, date);
        break;
      });
  }
  if (compression) {
    const char *enc = NULL;
    SMARTLIST_FOREACH(parsed_headers, const char *, s,
      if (!strcmpstart(s, "Content-Encoding: ")) {
        enc = s+18; break;
      });

    if (enc == NULL)
      *compression = NO_METHOD;
    else {
      *compression = compression_method_get_by_name(enc);

      if (*compression == UNKNOWN_METHOD)
        log_info(LD_HTTP, "Unrecognized content encoding: %s. Trying to deal.",
                 escaped(enc));
    }
  }
  SMARTLIST_FOREACH(parsed_headers, char *, s, tor_free(s));
  smartlist_free(parsed_headers);

  return 0;
}

/** If any directory object is arriving, and it's over 10MB large, we're
 * getting DoS'd.  (As of 0.1.2.x, raw directories are about 1MB, and we never
 * ask for more than 96 router descriptors at a time.)
 */
#define MAX_DIRECTORY_OBJECT_SIZE (10*(1<<20))

#define MAX_VOTE_DL_SIZE (MAX_DIRECTORY_OBJECT_SIZE * 5)

/** Read handler for directory connections.  (That's connections <em>to</em>
 * directory servers and connections <em>at</em> directory servers.)
 */
int
connection_dir_process_inbuf(dir_connection_t *conn)
{
  size_t max_size;
  tor_assert(conn);
  tor_assert(conn->base_.type == CONN_TYPE_DIR);

  /* Directory clients write, then read data until they receive EOF;
   * directory servers read data until they get an HTTP command, then
   * write their response (when it's finished flushing, they mark for
   * close).
   */

  /* If we're on the dirserver side, look for a command. */
  if (conn->base_.state == DIR_CONN_STATE_SERVER_COMMAND_WAIT) {
    if (directory_handle_command(conn) < 0) {
      connection_mark_for_close(TO_CONN(conn));
      return -1;
    }
    return 0;
  }

  max_size =
    (TO_CONN(conn)->purpose == DIR_PURPOSE_FETCH_STATUS_VOTE) ?
    MAX_VOTE_DL_SIZE : MAX_DIRECTORY_OBJECT_SIZE;

  if (connection_get_inbuf_len(TO_CONN(conn)) > max_size) {
    log_warn(LD_HTTP,
             "Too much data received from %s: "
             "denial of service attempt, or you need to upgrade?",
             connection_describe(TO_CONN(conn)));
    connection_mark_for_close(TO_CONN(conn));
    return -1;
  }

  if (!conn->base_.inbuf_reached_eof)
    log_debug(LD_HTTP,"Got data, not eof. Leaving on inbuf.");
  return 0;
}

/** Called when we're about to finally unlink and free a directory connection:
 * perform necessary accounting and cleanup */
void
connection_dir_about_to_close(dir_connection_t *dir_conn)
{
  connection_t *conn = TO_CONN(dir_conn);

  if (conn->state < DIR_CONN_STATE_CLIENT_FINISHED) {
    /* It's a directory connection and connecting or fetching
     * failed: forget about this router, and maybe try again. */
    connection_dir_client_request_failed(dir_conn);
  }

  connection_dir_client_refetch_hsdesc_if_needed(dir_conn);
}

/** Write handler for directory connections; called when all data has
 * been flushed.  Close the connection or wait for a response as
 * appropriate.
 */
int
connection_dir_finished_flushing(dir_connection_t *conn)
{
  tor_assert(conn);
  tor_assert(conn->base_.type == CONN_TYPE_DIR);

  if (conn->base_.marked_for_close)
    return 0;

  /* Note that we have finished writing the directory response. For direct
   * connections this means we're done; for tunneled connections it's only
   * an intermediate step. */
  if (conn->dirreq_id)
    geoip_change_dirreq_state(conn->dirreq_id, DIRREQ_TUNNELED,
                              DIRREQ_FLUSHING_DIR_CONN_FINISHED);
  else
    geoip_change_dirreq_state(TO_CONN(conn)->global_identifier,
                              DIRREQ_DIRECT,
                              DIRREQ_FLUSHING_DIR_CONN_FINISHED);
  switch (conn->base_.state) {
    case DIR_CONN_STATE_CONNECTING:
    case DIR_CONN_STATE_CLIENT_SENDING:
      log_debug(LD_DIR,"client finished sending command.");
      conn->base_.state = DIR_CONN_STATE_CLIENT_READING;
      return 0;
    case DIR_CONN_STATE_SERVER_WRITING:
      if (conn->spool) {
        log_warn(LD_BUG, "Emptied a dirserv buffer, but it's still spooling!");
        connection_mark_for_close(TO_CONN(conn));
      } else {
        log_debug(LD_DIRSERV, "Finished writing server response. Closing.");
        connection_mark_for_close(TO_CONN(conn));
      }
      return 0;
    default:
      log_warn(LD_BUG,"called in unexpected state %d.",
               conn->base_.state);
      tor_fragile_assert();
      return -1;
  }
  return 0;
}

/** Connected handler for directory connections: begin sending data to the
 * server, and return 0.
 * Only used when connections don't immediately connect. */
int
connection_dir_finished_connecting(dir_connection_t *conn)
{
  tor_assert(conn);
  tor_assert(conn->base_.type == CONN_TYPE_DIR);
  tor_assert(conn->base_.state == DIR_CONN_STATE_CONNECTING);

  log_debug(LD_HTTP,"Dir connection to %s established.",
            connection_describe_peer(TO_CONN(conn)));

  /* start flushing conn */
  conn->base_.state = DIR_CONN_STATE_CLIENT_SENDING;
  return 0;
}

/** Helper.  Compare two fp_pair_t objects, and return negative, 0, or
 * positive as appropriate. */
static int
compare_pairs_(const void **a, const void **b)
{
  const fp_pair_t *fp1 = *a, *fp2 = *b;
  int r;
  if ((r = fast_memcmp(fp1->first, fp2->first, DIGEST_LEN)))
    return r;
  else
    return fast_memcmp(fp1->second, fp2->second, DIGEST_LEN);
}

/** Divide a string <b>res</b> of the form FP1-FP2+FP3-FP4...[.z], where each
 * FP is a hex-encoded fingerprint, into a sequence of distinct sorted
 * fp_pair_t. Skip malformed pairs. On success, return 0 and add those
 * fp_pair_t into <b>pairs_out</b>.  On failure, return -1. */
int
dir_split_resource_into_fingerprint_pairs(const char *res,
                                          smartlist_t *pairs_out)
{
  smartlist_t *pairs_tmp = smartlist_new();
  smartlist_t *pairs_result = smartlist_new();

  smartlist_split_string(pairs_tmp, res, "+", 0, 0);
  if (smartlist_len(pairs_tmp)) {
    char *last = smartlist_get(pairs_tmp,smartlist_len(pairs_tmp)-1);
    size_t last_len = strlen(last);
    if (last_len > 2 && !strcmp(last+last_len-2, ".z")) {
      last[last_len-2] = '\0';
    }
  }
  SMARTLIST_FOREACH_BEGIN(pairs_tmp, char *, cp) {
    if (strlen(cp) != HEX_DIGEST_LEN*2+1) {
      log_info(LD_DIR,
             "Skipping digest pair %s with non-standard length.", escaped(cp));
    } else if (cp[HEX_DIGEST_LEN] != '-') {
      log_info(LD_DIR,
             "Skipping digest pair %s with missing dash.", escaped(cp));
    } else {
      fp_pair_t pair;
      if (base16_decode(pair.first, DIGEST_LEN,
                        cp, HEX_DIGEST_LEN) != DIGEST_LEN ||
          base16_decode(pair.second,DIGEST_LEN,
                        cp+HEX_DIGEST_LEN+1, HEX_DIGEST_LEN) != DIGEST_LEN) {
        log_info(LD_DIR, "Skipping non-decodable digest pair %s", escaped(cp));
      } else {
        smartlist_add(pairs_result, tor_memdup(&pair, sizeof(pair)));
      }
    }
    tor_free(cp);
  } SMARTLIST_FOREACH_END(cp);
  smartlist_free(pairs_tmp);

  /* Uniq-and-sort */
  smartlist_sort(pairs_result, compare_pairs_);
  smartlist_uniq(pairs_result, compare_pairs_, tor_free_);

  smartlist_add_all(pairs_out, pairs_result);
  smartlist_free(pairs_result);
  return 0;
}

/** Given a directory <b>resource</b> request, containing zero
 * or more strings separated by plus signs, followed optionally by ".z", store
 * the strings, in order, into <b>fp_out</b>.  If <b>compressed_out</b> is
 * non-NULL, set it to 1 if the resource ends in ".z", else set it to 0.
 *
 * If (flags & DSR_HEX), then delete all elements that aren't hex digests, and
 * decode the rest.  If (flags & DSR_BASE64), then use "-" rather than "+" as
 * a separator, delete all the elements that aren't base64-encoded digests,
 * and decode the rest.  If (flags & DSR_DIGEST256), these digests should be
 * 256 bits long; else they should be 160.
 *
 * If (flags & DSR_SORT_UNIQ), then sort the list and remove all duplicates.
 */
int
dir_split_resource_into_fingerprints(const char *resource,
                                     smartlist_t *fp_out, int *compressed_out,
                                     int flags)
{
  const int decode_hex = flags & DSR_HEX;
  const int decode_base64 = flags & DSR_BASE64;
  const int digests_are_256 = flags & DSR_DIGEST256;
  const int sort_uniq = flags & DSR_SORT_UNIQ;

  const int digest_len = digests_are_256 ? DIGEST256_LEN : DIGEST_LEN;
  const int hex_digest_len = digests_are_256 ?
    HEX_DIGEST256_LEN : HEX_DIGEST_LEN;
  const int base64_digest_len = digests_are_256 ?
    BASE64_DIGEST256_LEN : BASE64_DIGEST_LEN;
  smartlist_t *fp_tmp = smartlist_new();

  tor_assert(!(decode_hex && decode_base64));
  tor_assert(fp_out);

  smartlist_split_string(fp_tmp, resource, decode_base64?"-":"+", 0, 0);
  if (compressed_out)
    *compressed_out = 0;
  if (smartlist_len(fp_tmp)) {
    char *last = smartlist_get(fp_tmp,smartlist_len(fp_tmp)-1);
    size_t last_len = strlen(last);
    if (last_len > 2 && !strcmp(last+last_len-2, ".z")) {
      last[last_len-2] = '\0';
      if (compressed_out)
        *compressed_out = 1;
    }
  }
  if (decode_hex || decode_base64) {
    const size_t encoded_len = decode_hex ? hex_digest_len : base64_digest_len;
    int i;
    char *cp, *d = NULL;
    for (i = 0; i < smartlist_len(fp_tmp); ++i) {
      cp = smartlist_get(fp_tmp, i);
      if (strlen(cp) != encoded_len) {
        log_info(LD_DIR,
                 "Skipping digest %s with non-standard length.", escaped(cp));
        smartlist_del_keeporder(fp_tmp, i--);
        goto again;
      }
      d = tor_malloc_zero(digest_len);
      if (decode_hex ?
          (base16_decode(d, digest_len, cp, hex_digest_len) != digest_len) :
          (base64_decode(d, digest_len, cp, base64_digest_len)
                         != digest_len)) {
          log_info(LD_DIR, "Skipping non-decodable digest %s", escaped(cp));
          smartlist_del_keeporder(fp_tmp, i--);
          goto again;
      }
      smartlist_set(fp_tmp, i, d);
      d = NULL;
    again:
      tor_free(cp);
      tor_free(d);
    }
  }
  if (sort_uniq) {
    if (decode_hex || decode_base64) {
      if (digests_are_256) {
        smartlist_sort_digests256(fp_tmp);
        smartlist_uniq_digests256(fp_tmp);
      } else {
        smartlist_sort_digests(fp_tmp);
        smartlist_uniq_digests(fp_tmp);
      }
    } else {
      smartlist_sort_strings(fp_tmp);
      smartlist_uniq_strings(fp_tmp);
    }
  }
  smartlist_add_all(fp_out, fp_tmp);
  smartlist_free(fp_tmp);
  return 0;
}
