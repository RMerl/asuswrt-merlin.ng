/* Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file control_fmt.c
 * \brief Formatting functions for controller data.
 */

#include "core/or/or.h"

#include "core/mainloop/connection.h"
#include "core/or/circuitbuild.h"
#include "core/or/circuitlist.h"
#include "core/or/connection_edge.h"
#include "feature/control/control_fmt.h"
#include "feature/control/control_proto.h"
#include "feature/nodelist/nodelist.h"

#include "core/or/cpath_build_state_st.h"
#include "core/or/entry_connection_st.h"
#include "core/or/or_connection_st.h"
#include "core/or/origin_circuit_st.h"
#include "core/or/socks_request_st.h"
#include "feature/control/control_connection_st.h"

/** Given an AP connection <b>conn</b> and a <b>len</b>-character buffer
 * <b>buf</b>, determine the address:port combination requested on
 * <b>conn</b>, and write it to <b>buf</b>.  Return 0 on success, -1 on
 * failure. */
int
write_stream_target_to_buf(entry_connection_t *conn, char *buf, size_t len)
{
  char buf2[256];
  if (conn->chosen_exit_name)
    if (tor_snprintf(buf2, sizeof(buf2), ".%s.exit", conn->chosen_exit_name)<0)
      return -1;
  if (!conn->socks_request)
    return -1;
  if (tor_snprintf(buf, len, "%s%s%s:%d",
               conn->socks_request->address,
               conn->chosen_exit_name ? buf2 : "",
               !conn->chosen_exit_name && connection_edge_is_rendezvous_stream(
                                     ENTRY_TO_EDGE_CONN(conn)) ? ".onion" : "",
               conn->socks_request->port)<0)
    return -1;
  return 0;
}

/** Figure out the best name for the target router of an OR connection
 * <b>conn</b>, and write it into the <b>len</b>-character buffer
 * <b>name</b>. */
void
orconn_target_get_name(char *name, size_t len, or_connection_t *conn)
{
  const node_t *node = node_get_by_id(conn->identity_digest);
  if (node) {
    tor_assert(len > MAX_VERBOSE_NICKNAME_LEN);
    node_get_verbose_nickname(node, name);
  } else if (! tor_digest_is_zero(conn->identity_digest)) {
    name[0] = '$';
    base16_encode(name+1, len-1, conn->identity_digest,
                  DIGEST_LEN);
  } else {
    tor_snprintf(name, len, "%s:%d",
                 conn->base_.address, conn->base_.port);
  }
}

/** Allocate and return a description of <b>circ</b>'s current status,
 * including its path (if any). */
char *
circuit_describe_status_for_controller(origin_circuit_t *circ)
{
  char *rv;
  smartlist_t *descparts = smartlist_new();

  {
    char *vpath = circuit_list_path_for_controller(circ);
    if (*vpath) {
      smartlist_add(descparts, vpath);
    } else {
      tor_free(vpath); /* empty path; don't put an extra space in the result */
    }
  }

  {
    cpath_build_state_t *build_state = circ->build_state;
    smartlist_t *flaglist = smartlist_new();
    char *flaglist_joined;

    if (build_state->onehop_tunnel)
      smartlist_add(flaglist, (void *)"ONEHOP_TUNNEL");
    if (build_state->is_internal)
      smartlist_add(flaglist, (void *)"IS_INTERNAL");
    if (build_state->need_capacity)
      smartlist_add(flaglist, (void *)"NEED_CAPACITY");
    if (build_state->need_uptime)
      smartlist_add(flaglist, (void *)"NEED_UPTIME");

    /* Only emit a BUILD_FLAGS argument if it will have a non-empty value. */
    if (smartlist_len(flaglist)) {
      flaglist_joined = smartlist_join_strings(flaglist, ",", 0, NULL);

      smartlist_add_asprintf(descparts, "BUILD_FLAGS=%s", flaglist_joined);

      tor_free(flaglist_joined);
    }

    smartlist_free(flaglist);
  }

  smartlist_add_asprintf(descparts, "PURPOSE=%s",
                    circuit_purpose_to_controller_string(circ->base_.purpose));

  {
    const char *hs_state =
      circuit_purpose_to_controller_hs_state_string(circ->base_.purpose);

    if (hs_state != NULL) {
      smartlist_add_asprintf(descparts, "HS_STATE=%s", hs_state);
    }
  }

  if (circ->rend_data != NULL || circ->hs_ident != NULL) {
    char addr[HS_SERVICE_ADDR_LEN_BASE32 + 1];
    const char *onion_address;
    if (circ->rend_data) {
      onion_address = rend_data_get_address(circ->rend_data);
    } else {
      hs_build_address(&circ->hs_ident->identity_pk, HS_VERSION_THREE, addr);
      onion_address = addr;
    }
    smartlist_add_asprintf(descparts, "REND_QUERY=%s", onion_address);
  }

  {
    char tbuf[ISO_TIME_USEC_LEN+1];
    format_iso_time_nospace_usec(tbuf, &circ->base_.timestamp_created);

    smartlist_add_asprintf(descparts, "TIME_CREATED=%s", tbuf);
  }

  // Show username and/or password if available.
  if (circ->socks_username_len > 0) {
    char* socks_username_escaped = esc_for_log_len(circ->socks_username,
                                     (size_t) circ->socks_username_len);
    smartlist_add_asprintf(descparts, "SOCKS_USERNAME=%s",
                           socks_username_escaped);
    tor_free(socks_username_escaped);
  }
  if (circ->socks_password_len > 0) {
    char* socks_password_escaped = esc_for_log_len(circ->socks_password,
                                     (size_t) circ->socks_password_len);
    smartlist_add_asprintf(descparts, "SOCKS_PASSWORD=%s",
                           socks_password_escaped);
    tor_free(socks_password_escaped);
  }

  rv = smartlist_join_strings(descparts, " ", 0, NULL);

  SMARTLIST_FOREACH(descparts, char *, cp, tor_free(cp));
  smartlist_free(descparts);

  return rv;
}

/** Return a longname the node whose identity is <b>id_digest</b>. If
 * node_get_by_id() returns NULL, base 16 encoding of <b>id_digest</b> is
 * returned instead.
 *
 * This function is not thread-safe.  Each call to this function invalidates
 * previous values returned by this function.
 */
MOCK_IMPL(const char *,
node_describe_longname_by_id,(const char *id_digest))
{
  static char longname[MAX_VERBOSE_NICKNAME_LEN+1];
  node_get_verbose_nickname_by_id(id_digest, longname);
  return longname;
}
