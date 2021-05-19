/* Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file rendclient.c
 * \brief Client code to access location-hidden services.
 **/

#include "core/or/or.h"
#include "app/config/config.h"
#include "core/mainloop/connection.h"
#include "core/mainloop/mainloop.h"
#include "core/or/circuitbuild.h"
#include "core/or/circuitlist.h"
#include "core/or/circuituse.h"
#include "core/or/connection_edge.h"
#include "core/or/extendinfo.h"
#include "core/or/relay.h"
#include "feature/client/circpathbias.h"
#include "feature/control/control_events.h"
#include "feature/dirclient/dirclient.h"
#include "feature/dircommon/directory.h"
#include "feature/hs/hs_circuit.h"
#include "feature/hs/hs_client.h"
#include "feature/hs/hs_common.h"
#include "feature/nodelist/describe.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerlist.h"
#include "feature/nodelist/routerset.h"
#include "feature/rend/rendclient.h"
#include "feature/rend/rendcommon.h"
#include "feature/stats/rephist.h"
#include "lib/crypt_ops/crypto_dh.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/crypt_ops/crypto_util.h"
#include "lib/encoding/confline.h"

#include "core/or/cpath_build_state_st.h"
#include "core/or/crypt_path_st.h"
#include "feature/dircommon/dir_connection_st.h"
#include "core/or/entry_connection_st.h"
#include "core/or/extend_info_st.h"
#include "core/or/origin_circuit_st.h"
#include "feature/rend/rend_intro_point_st.h"
#include "feature/rend/rend_service_descriptor_st.h"
#include "feature/nodelist/routerstatus_st.h"

static extend_info_t *rend_client_get_random_intro_impl(
                          const rend_cache_entry_t *rend_query,
                          const int strict, const int warnings);

/** Purge all potentially remotely-detectable state held in the hidden
 * service client code.  Called on SIGNAL NEWNYM. */
void
rend_client_purge_state(void)
{
  rend_cache_purge();
  rend_cache_failure_purge();
  rend_client_cancel_descriptor_fetches();
  hs_purge_last_hid_serv_requests();
}

/** Called when we've established a circuit to an introduction point:
 * send the introduction request. */
void
rend_client_introcirc_has_opened(origin_circuit_t *circ)
{
  tor_assert(circ->base_.purpose == CIRCUIT_PURPOSE_C_INTRODUCING);
  tor_assert(circ->cpath);

  log_info(LD_REND,"introcirc is open");
  connection_ap_attach_pending(1);
}

/** Send the establish-rendezvous cell along a rendezvous circuit. if
 * it fails, mark the circ for close and return -1. else return 0.
 */
static int
rend_client_send_establish_rendezvous(origin_circuit_t *circ)
{
  tor_assert(circ->base_.purpose == CIRCUIT_PURPOSE_C_ESTABLISH_REND);
  tor_assert(circ->rend_data);
  log_info(LD_REND, "Sending an ESTABLISH_RENDEZVOUS cell");

  crypto_rand(circ->rend_data->rend_cookie, REND_COOKIE_LEN);

  /* Set timestamp_dirty, because circuit_expire_building expects it,
   * and the rend cookie also means we've used the circ. */
  circ->base_.timestamp_dirty = time(NULL);

  /* We've attempted to use this circuit. Probe it if we fail */
  pathbias_count_use_attempt(circ);

  if (relay_send_command_from_edge(0, TO_CIRCUIT(circ),
                                   RELAY_COMMAND_ESTABLISH_RENDEZVOUS,
                                   circ->rend_data->rend_cookie,
                                   REND_COOKIE_LEN,
                                   circ->cpath->prev)<0) {
    /* circ is already marked for close */
    log_warn(LD_GENERAL, "Couldn't send ESTABLISH_RENDEZVOUS cell");
    return -1;
  }

  return 0;
}

/** Called when we're trying to connect an ap conn; sends an INTRODUCE1 cell
 * down introcirc if possible.
 */
int
rend_client_send_introduction(origin_circuit_t *introcirc,
                              origin_circuit_t *rendcirc)
{
  const or_options_t *options = get_options();
  size_t payload_len;
  int r, v3_shift = 0;
  char payload[RELAY_PAYLOAD_SIZE];
  char tmp[RELAY_PAYLOAD_SIZE];
  rend_cache_entry_t *entry = NULL;
  crypt_path_t *cpath;
  ptrdiff_t dh_offset;
  crypto_pk_t *intro_key = NULL;
  int status = 0;
  const char *onion_address;

  tor_assert(introcirc->base_.purpose == CIRCUIT_PURPOSE_C_INTRODUCING);
  tor_assert(rendcirc->base_.purpose == CIRCUIT_PURPOSE_C_REND_READY);
  tor_assert(introcirc->rend_data);
  tor_assert(rendcirc->rend_data);
  tor_assert(!rend_cmp_service_ids(rend_data_get_address(introcirc->rend_data),
                                  rend_data_get_address(rendcirc->rend_data)));
  assert_circ_anonymity_ok(introcirc, options);
  assert_circ_anonymity_ok(rendcirc, options);
  onion_address = rend_data_get_address(introcirc->rend_data);

  r = rend_cache_lookup_entry(onion_address, -1, &entry);
  /* An invalid onion address is not possible else we have a big issue. */
  tor_assert(r != -EINVAL);
  if (r < 0 || !rend_client_any_intro_points_usable(entry)) {
    /* If the descriptor is not found or the intro points are not usable
     * anymore, trigger a fetch. */
    log_info(LD_REND,
             "query %s didn't have valid rend desc in cache. "
             "Refetching descriptor.",
             safe_str_client(onion_address));
    rend_client_refetch_v2_renddesc(introcirc->rend_data);
    {
      connection_t *conn;

      while ((conn = connection_get_by_type_state_rendquery(CONN_TYPE_AP,
                       AP_CONN_STATE_CIRCUIT_WAIT, onion_address))) {
        connection_ap_mark_as_waiting_for_renddesc(TO_ENTRY_CONN(conn));
      }
    }

    status = -1;
    goto cleanup;
  }

  /* first 20 bytes of payload are the hash of the service's pk */
  intro_key = NULL;
  SMARTLIST_FOREACH(entry->parsed->intro_nodes, rend_intro_point_t *,
                    intro, {
    if (tor_memeq(introcirc->build_state->chosen_exit->identity_digest,
                intro->extend_info->identity_digest, DIGEST_LEN)) {
      intro_key = intro->intro_key;
      break;
    }
  });
  if (!intro_key) {
    log_info(LD_REND, "Could not find intro key for %s at %s; we "
             "have a v2 rend desc with %d intro points. "
             "Trying a different intro point...",
             safe_str_client(onion_address),
             safe_str_client(extend_info_describe(
                                   introcirc->build_state->chosen_exit)),
             smartlist_len(entry->parsed->intro_nodes));

    if (hs_client_reextend_intro_circuit(introcirc)) {
      status = -2;
      goto perm_err;
    } else {
      status = -1;
      goto cleanup;
    }
  }
  if (crypto_pk_get_digest(intro_key, payload)<0) {
    log_warn(LD_BUG, "Internal error: couldn't hash public key.");
    status = -2;
    goto perm_err;
  }

  /* Initialize the pending_final_cpath and start the DH handshake. */
  cpath = rendcirc->build_state->pending_final_cpath;
  if (!cpath) {
    cpath = rendcirc->build_state->pending_final_cpath =
      tor_malloc_zero(sizeof(crypt_path_t));
    cpath->magic = CRYPT_PATH_MAGIC;
    if (!(cpath->rend_dh_handshake_state = crypto_dh_new(DH_TYPE_REND))) {
      log_warn(LD_BUG, "Internal error: couldn't allocate DH.");
      status = -2;
      goto perm_err;
    }
    if (crypto_dh_generate_public(cpath->rend_dh_handshake_state)<0) {
      log_warn(LD_BUG, "Internal error: couldn't generate g^x.");
      status = -2;
      goto perm_err;
    }
  }

  /* If version is 3, write (optional) auth data and timestamp. */
  if (entry->parsed->protocols & (1<<3)) {
    tmp[0] = 3; /* version 3 of the cell format */
    /* auth type, if any */
    tmp[1] = (uint8_t) TO_REND_DATA_V2(introcirc->rend_data)->auth_type;
    v3_shift = 1;
    if (tmp[1] != REND_NO_AUTH) {
      set_uint16(tmp+2, htons(REND_DESC_COOKIE_LEN));
      memcpy(tmp+4, TO_REND_DATA_V2(introcirc->rend_data)->descriptor_cookie,
             REND_DESC_COOKIE_LEN);
      v3_shift += 2+REND_DESC_COOKIE_LEN;
    }
    /* Once this held a timestamp. */
    set_uint32(tmp+v3_shift+1, 0);
    v3_shift += 4;
  } /* if version 2 only write version number */
  else if (entry->parsed->protocols & (1<<2)) {
    tmp[0] = 2; /* version 2 of the cell format */
  }

  /* write the remaining items into tmp */
  if (entry->parsed->protocols & (1<<3) || entry->parsed->protocols & (1<<2)) {
    /* version 2 format */
    extend_info_t *extend_info = rendcirc->build_state->chosen_exit;
    int klen;
    const tor_addr_port_t *orport =
      extend_info_get_orport(extend_info, AF_INET);
    IF_BUG_ONCE(! orport) {
      /* we should never put an IPv6 address here. */
      goto perm_err;
    }
    /* nul pads */
    set_uint32(tmp+v3_shift+1, tor_addr_to_ipv4n(&orport->addr));
    set_uint16(tmp+v3_shift+5, htons(orport->port));
    memcpy(tmp+v3_shift+7, extend_info->identity_digest, DIGEST_LEN);
    klen = crypto_pk_asn1_encode(extend_info->onion_key,
                                 tmp+v3_shift+7+DIGEST_LEN+2,
                                 sizeof(tmp)-(v3_shift+7+DIGEST_LEN+2));
    if (klen < 0) {
      log_warn(LD_BUG,"Internal error: can't encode public key.");
      status = -2;
      goto perm_err;
    }
    set_uint16(tmp+v3_shift+7+DIGEST_LEN, htons(klen));
    memcpy(tmp+v3_shift+7+DIGEST_LEN+2+klen, rendcirc->rend_data->rend_cookie,
           REND_COOKIE_LEN);
    dh_offset = v3_shift+7+DIGEST_LEN+2+klen+REND_COOKIE_LEN;
  } else {
    /* Version 0. */

    /* Some compilers are smart enough to work out that nickname can be more
     * than 19 characters, when it's a hexdigest. They warn that strncpy()
     * will truncate hexdigests without NUL-terminating them. But we only put
     * hexdigests in HSDir and general circuit exits. */
    if (BUG(strlen(rendcirc->build_state->chosen_exit->nickname)
            > MAX_NICKNAME_LEN)) {
      goto perm_err;
    }
    strlcpy(tmp, rendcirc->build_state->chosen_exit->nickname,
            sizeof(tmp));
    memcpy(tmp+MAX_NICKNAME_LEN+1, rendcirc->rend_data->rend_cookie,
           REND_COOKIE_LEN);
    dh_offset = MAX_NICKNAME_LEN+1+REND_COOKIE_LEN;
  }

  if (crypto_dh_get_public(cpath->rend_dh_handshake_state, tmp+dh_offset,
                           DH1024_KEY_LEN)<0) {
    log_warn(LD_BUG, "Internal error: couldn't extract g^x.");
    status = -2;
    goto perm_err;
  }

  /*XXX maybe give crypto_pk_obsolete_public_hybrid_encrypt a max_len arg,
   * to avoid buffer overflows? */
  r = crypto_pk_obsolete_public_hybrid_encrypt(intro_key, payload+DIGEST_LEN,
                                      sizeof(payload)-DIGEST_LEN,
                                      tmp,
                                      (int)(dh_offset+DH1024_KEY_LEN),
                                      PK_PKCS1_OAEP_PADDING, 0);
  if (r<0) {
    log_warn(LD_BUG,"Internal error: hybrid pk encrypt failed.");
    status = -2;
    goto perm_err;
  }

  payload_len = DIGEST_LEN + r;
  tor_assert(payload_len <= RELAY_PAYLOAD_SIZE); /* we overran something */

  /* Copy the rendezvous cookie from rendcirc to introcirc, so that
   * when introcirc gets an ack, we can change the state of the right
   * rendezvous circuit. */
  memcpy(introcirc->rend_data->rend_cookie, rendcirc->rend_data->rend_cookie,
         REND_COOKIE_LEN);

  log_info(LD_REND, "Sending an INTRODUCE1 cell");
  if (relay_send_command_from_edge(0, TO_CIRCUIT(introcirc),
                                   RELAY_COMMAND_INTRODUCE1,
                                   payload, payload_len,
                                   introcirc->cpath->prev)<0) {
    /* introcirc is already marked for close. leave rendcirc alone. */
    log_warn(LD_BUG, "Couldn't send INTRODUCE1 cell");
    status = -2;
    goto cleanup;
  }

  /* Now, we wait for an ACK or NAK on this circuit. */
  circuit_change_purpose(TO_CIRCUIT(introcirc),
                         CIRCUIT_PURPOSE_C_INTRODUCE_ACK_WAIT);
  /* Set timestamp_dirty, because circuit_expire_building expects it
   * to specify when a circuit entered the _C_INTRODUCE_ACK_WAIT
   * state. */
  introcirc->base_.timestamp_dirty = time(NULL);

  pathbias_count_use_attempt(introcirc);

  goto cleanup;

 perm_err:
  if (!introcirc->base_.marked_for_close)
    circuit_mark_for_close(TO_CIRCUIT(introcirc), END_CIRC_REASON_INTERNAL);
  circuit_mark_for_close(TO_CIRCUIT(rendcirc), END_CIRC_REASON_INTERNAL);
 cleanup:
  memwipe(payload, 0, sizeof(payload));
  memwipe(tmp, 0, sizeof(tmp));

  return status;
}

/** Called when a rendezvous circuit is open; sends a establish
 * rendezvous circuit as appropriate. */
void
rend_client_rendcirc_has_opened(origin_circuit_t *circ)
{
  tor_assert(circ->base_.purpose == CIRCUIT_PURPOSE_C_ESTABLISH_REND);

  log_info(LD_REND,"rendcirc is open");

  /* generate a rendezvous cookie, store it in circ */
  if (rend_client_send_establish_rendezvous(circ) < 0) {
    return;
  }
}

/**
 * Called to close other intro circuits we launched in parallel.
 */
static void
rend_client_close_other_intros(const uint8_t *rend_pk_digest)
{
  /* abort parallel intro circs, if any */
  SMARTLIST_FOREACH_BEGIN(circuit_get_global_list(), circuit_t *, c) {
    if ((c->purpose == CIRCUIT_PURPOSE_C_INTRODUCING ||
        c->purpose == CIRCUIT_PURPOSE_C_INTRODUCE_ACK_WAIT) &&
        !c->marked_for_close && CIRCUIT_IS_ORIGIN(c)) {
      origin_circuit_t *oc = TO_ORIGIN_CIRCUIT(c);
      if (oc->rend_data &&
          rend_circuit_pk_digest_eq(oc, rend_pk_digest)) {
        log_info(LD_REND|LD_CIRC, "Closing introduction circuit %d that we "
                 "built in parallel (Purpose %d).", oc->global_identifier,
                 c->purpose);
        circuit_mark_for_close(c, END_CIRC_REASON_IP_NOW_REDUNDANT);
      }
    }
  }
  SMARTLIST_FOREACH_END(c);
}

/** Called when get an ACK or a NAK for a REND_INTRODUCE1 cell.
 */
int
rend_client_introduction_acked(origin_circuit_t *circ,
                               const uint8_t *request, size_t request_len)
{
  const or_options_t *options = get_options();
  origin_circuit_t *rendcirc;
  (void) request; // XXXX Use this.

  tor_assert(circ->build_state);
  tor_assert(circ->build_state->chosen_exit);
  assert_circ_anonymity_ok(circ, options);
  tor_assert(circ->rend_data);

  if (request_len == 0) {
    /* It's an ACK; the introduction point relayed our introduction request. */
    /* Locate the rend circ which is waiting to hear about this ack,
     * and tell it.
     */
    log_info(LD_REND,"Received ack. Telling rend circ...");
    rendcirc = circuit_get_ready_rend_circ_by_rend_data(circ->rend_data);
    if (rendcirc) { /* remember the ack */
      assert_circ_anonymity_ok(rendcirc, options);
      circuit_change_purpose(TO_CIRCUIT(rendcirc),
                             CIRCUIT_PURPOSE_C_REND_READY_INTRO_ACKED);
      /* Set timestamp_dirty, because circuit_expire_building expects
       * it to specify when a circuit entered the
       * _C_REND_READY_INTRO_ACKED state. */
      rendcirc->base_.timestamp_dirty = time(NULL);
    } else {
      log_info(LD_REND,"...Found no rend circ. Dropping on the floor.");
    }
    /* Save the rend data digest to a temporary object so that we don't access
     * it after we mark the circuit for close. */
    const uint8_t *rend_digest_tmp = NULL;
    size_t digest_len;
    uint8_t *cached_rend_digest = NULL;
    rend_digest_tmp = rend_data_get_pk_digest(circ->rend_data, &digest_len);
    cached_rend_digest = tor_malloc_zero(digest_len);
    memcpy(cached_rend_digest, rend_digest_tmp, digest_len);

    /* close the circuit: we won't need it anymore. */
    circuit_change_purpose(TO_CIRCUIT(circ),
                           CIRCUIT_PURPOSE_C_INTRODUCE_ACKED);
    circuit_mark_for_close(TO_CIRCUIT(circ), END_CIRC_REASON_FINISHED);

    /* close any other intros launched in parallel */
    rend_client_close_other_intros(cached_rend_digest);
    tor_free(cached_rend_digest); /* free the temporary digest */
  } else {
    /* It's a NAK; the introduction point didn't relay our request. */
    circuit_change_purpose(TO_CIRCUIT(circ), CIRCUIT_PURPOSE_C_INTRODUCING);
    /* Remove this intro point from the set of viable introduction
     * points. If any remain, extend to a new one and try again.
     * If none remain, refetch the service descriptor.
     */
    log_info(LD_REND, "Got nack for %s from %s...",
        safe_str_client(rend_data_get_address(circ->rend_data)),
        safe_str_client(extend_info_describe(circ->build_state->chosen_exit)));
    if (rend_client_report_intro_point_failure(circ->build_state->chosen_exit,
                                             circ->rend_data,
                                             INTRO_POINT_FAILURE_GENERIC)>0) {
      /* There are introduction points left. Re-extend the circuit to
       * another intro point and try again. */
      int result = hs_client_reextend_intro_circuit(circ);
      /* XXXX If that call failed, should we close the rend circuit,
       * too? */
      return result;
    } else {
      /* Close circuit because no more intro points are usable thus not
       * useful anymore. Change it's purpose before so we don't report an
       * intro point failure again triggering an extra descriptor fetch. */
      circuit_change_purpose(TO_CIRCUIT(circ),
          CIRCUIT_PURPOSE_C_INTRODUCE_ACKED);
      circuit_mark_for_close(TO_CIRCUIT(circ), END_CIRC_REASON_FINISHED);
    }
  }
  return 0;
}

/** Determine the responsible hidden service directories for <b>desc_id</b>
 * and fetch the descriptor with that ID from one of them. Only
 * send a request to a hidden service directory that we have not yet tried
 * during this attempt to connect to this hidden service; on success, return 1,
 * in the case that no hidden service directory is left to ask for the
 * descriptor, return 0, and in case of a failure -1.  */
static int
directory_get_from_hs_dir(const char *desc_id,
                          const rend_data_t *rend_query,
                          routerstatus_t *rs_hsdir)
{
  routerstatus_t *hs_dir = rs_hsdir;
  char *hsdir_fp;
  char desc_id_base32[REND_DESC_ID_V2_LEN_BASE32 + 1];
  char descriptor_cookie_base64[3*REND_DESC_COOKIE_LEN_BASE64];
  const rend_data_v2_t *rend_data;
  const int how_to_fetch = DIRIND_ANONYMOUS;

  tor_assert(desc_id);
  tor_assert(rend_query);
  rend_data = TO_REND_DATA_V2(rend_query);

  base32_encode(desc_id_base32, sizeof(desc_id_base32),
                desc_id, DIGEST_LEN);

  /* Automatically pick an hs dir if none given. */
  if (!rs_hsdir) {
    bool rate_limited = false;

    /* Determine responsible dirs. Even if we can't get all we want, work with
     * the ones we have. If it's empty, we'll notice in hs_pick_hsdir(). */
    smartlist_t *responsible_dirs = smartlist_new();
    hid_serv_get_responsible_directories(responsible_dirs, desc_id);

    hs_dir = hs_pick_hsdir(responsible_dirs, desc_id_base32, &rate_limited);
    if (!hs_dir) {
      /* No suitable hs dir can be found, stop right now. */
      const char *query_response = (rate_limited) ? "QUERY_RATE_LIMITED" :
                                                    "QUERY_NO_HSDIR";
      control_event_hsv2_descriptor_failed(rend_query, NULL, query_response);
      control_event_hs_descriptor_content(rend_data_get_address(rend_query),
                                          desc_id_base32, NULL, NULL);
      return 0;
    }
  }

  /* Add a copy of the HSDir identity digest to the query so we can track it
   * on the control port. */
  hsdir_fp = tor_memdup(hs_dir->identity_digest,
                        sizeof(hs_dir->identity_digest));
  smartlist_add(rend_query->hsdirs_fp, hsdir_fp);

  /* Encode descriptor cookie for logging purposes. Also, if the cookie is
   * malformed, no fetch is triggered thus this needs to be done before the
   * fetch request. */
  if (rend_data->auth_type != REND_NO_AUTH) {
    if (base64_encode(descriptor_cookie_base64,
                      sizeof(descriptor_cookie_base64),
                      rend_data->descriptor_cookie,
                      REND_DESC_COOKIE_LEN,
                      0)<0) {
      log_warn(LD_BUG, "Could not base64-encode descriptor cookie.");
      control_event_hsv2_descriptor_failed(rend_query, hsdir_fp, "BAD_DESC");
      control_event_hs_descriptor_content(rend_data_get_address(rend_query),
                                          desc_id_base32, hsdir_fp, NULL);
      return 0;
    }
    /* Remove == signs. */
    descriptor_cookie_base64[strlen(descriptor_cookie_base64)-2] = '\0';
  } else {
    strlcpy(descriptor_cookie_base64, "(none)",
            sizeof(descriptor_cookie_base64));
  }

  /* Send fetch request. (Pass query and possibly descriptor cookie so that
   * they can be written to the directory connection and be referred to when
   * the response arrives. */
  directory_request_t *req =
    directory_request_new(DIR_PURPOSE_FETCH_RENDDESC_V2);
  directory_request_set_routerstatus(req, hs_dir);
  directory_request_set_indirection(req, how_to_fetch);
  directory_request_set_resource(req, desc_id_base32);
  directory_request_set_rend_query(req, rend_query);
  directory_initiate_request(req);
  directory_request_free(req);

  log_info(LD_REND, "Sending fetch request for v2 descriptor for "
                    "service '%s' with descriptor ID '%s', auth type %d, "
                    "and descriptor cookie '%s' to hidden service "
                    "directory %s",
           rend_data->onion_address, desc_id_base32,
           rend_data->auth_type,
           (rend_data->auth_type == REND_NO_AUTH ? "[none]" :
            escaped_safe_str_client(descriptor_cookie_base64)),
           routerstatus_describe(hs_dir));
  control_event_hs_descriptor_requested(rend_data->onion_address,
                                        rend_data->auth_type,
                                        hs_dir->identity_digest,
                                        desc_id_base32, NULL);
  return 1;
}

/** Remove tracked HSDir requests from our history for this hidden service
 *  descriptor <b>desc_id</b> (of size DIGEST_LEN) */
static void
purge_v2_hidserv_req(const char *desc_id)
{
  char desc_id_base32[REND_DESC_ID_V2_LEN_BASE32 + 1];

  /* The hsdir request tracker stores v2 keys using the base32 encoded
     desc_id. Do it: */
  base32_encode(desc_id_base32, sizeof(desc_id_base32), desc_id,
                DIGEST_LEN);
  hs_purge_hid_serv_from_last_hid_serv_requests(desc_id_base32);
}

/** Fetch a v2 descriptor using the given descriptor id. If any hsdir(s) are
 * given, they will be used instead.
 *
 * On success, 1 is returned. If no hidden service is left to ask, return 0.
 * On error, -1 is returned. */
static int
fetch_v2_desc_by_descid(const char *desc_id,
                        const rend_data_t *rend_query, smartlist_t *hsdirs)
{
  int ret;

  tor_assert(rend_query);

  if (!hsdirs) {
    ret = directory_get_from_hs_dir(desc_id, rend_query, NULL);
    goto end; /* either success or failure, but we're done */
  }

  /* Using the given hsdir list, trigger a fetch on each of them. */
  SMARTLIST_FOREACH_BEGIN(hsdirs, routerstatus_t *, hs_dir) {
    /* This should always be a success. */
    ret = directory_get_from_hs_dir(desc_id, rend_query, hs_dir);
    tor_assert(ret);
  } SMARTLIST_FOREACH_END(hs_dir);

  /* Everything went well. */
  ret = 0;

 end:
  return ret;
}

/** Fetch a v2 descriptor using the onion address in the given query object.
 * This will compute the descriptor id for each replicas and fetch it on the
 * given hsdir(s) if any or the responsible ones that are chosen
 * automatically.
 *
 * On success, 1 is returned. If no hidden service is left to ask, return 0.
 * On error, -1 is returned. */
static int
fetch_v2_desc_by_addr(rend_data_t *rend_query, smartlist_t *hsdirs)
{
  char descriptor_id[DIGEST_LEN];
  int replicas_left_to_try[REND_NUMBER_OF_NON_CONSECUTIVE_REPLICAS];
  int i, tries_left, ret;
  rend_data_v2_t *rend_data = TO_REND_DATA_V2(rend_query);

  /* Randomly iterate over the replicas until a descriptor can be fetched
   * from one of the consecutive nodes, or no options are left. */
  for (i = 0; i < REND_NUMBER_OF_NON_CONSECUTIVE_REPLICAS; i++) {
    replicas_left_to_try[i] = i;
  }

  tries_left = REND_NUMBER_OF_NON_CONSECUTIVE_REPLICAS;
  while (tries_left > 0) {
    int rand_val = crypto_rand_int(tries_left);
    int chosen_replica = replicas_left_to_try[rand_val];
    replicas_left_to_try[rand_val] = replicas_left_to_try[--tries_left];

    ret = rend_compute_v2_desc_id(descriptor_id,
                                  rend_data->onion_address,
                                  rend_data->auth_type == REND_STEALTH_AUTH ?
                                    rend_data->descriptor_cookie : NULL,
                                  time(NULL), chosen_replica);
    if (ret < 0) {
      /* Normally, on failure the descriptor_id is untouched but let's be
       * safe in general in case the function changes at some point. */
      goto end;
    }

    if (tor_memcmp(descriptor_id, rend_data->descriptor_id[chosen_replica],
                   sizeof(descriptor_id)) != 0) {
      /* Not equal from what we currently have so purge the last hid serv
       * request cache and update the descriptor ID with the new value. */
      purge_v2_hidserv_req(rend_data->descriptor_id[chosen_replica]);
      memcpy(rend_data->descriptor_id[chosen_replica], descriptor_id,
             sizeof(rend_data->descriptor_id[chosen_replica]));
    }

    /* Trigger the fetch with the computed descriptor ID. */
    ret = fetch_v2_desc_by_descid(descriptor_id, rend_query, hsdirs);
    if (ret != 0) {
      /* Either on success or failure, as long as we tried a fetch we are
       * done here. */
      goto end;
    }
  }

  /* If we come here, there are no hidden service directories left. */
  log_info(LD_REND, "Could not pick one of the responsible hidden "
                    "service directories to fetch descriptors, because "
                    "we already tried them all unsuccessfully.");
  ret = 0;

 end:
  memwipe(descriptor_id, 0, sizeof(descriptor_id));
  return ret;
}

/** Fetch a v2 descriptor using the given query. If any hsdir are specified,
 * use them for the fetch.
 *
 * On success, 1 is returned. If no hidden service is left to ask, return 0.
 * On error, -1 is returned. */
int
rend_client_fetch_v2_desc(rend_data_t *query, smartlist_t *hsdirs)
{
  int ret;
  rend_data_v2_t *rend_data;
  const char *onion_address;

  tor_assert(query);

  /* Get the version 2 data structure of the query. */
  rend_data = TO_REND_DATA_V2(query);
  onion_address = rend_data_get_address(query);

  /* Depending on what's available in the rend data query object, we will
   * trigger a fetch by HS address or using a descriptor ID. */

  if (onion_address[0] != '\0') {
    ret = fetch_v2_desc_by_addr(query, hsdirs);
  } else if (!tor_digest_is_zero(rend_data->desc_id_fetch)) {
    ret = fetch_v2_desc_by_descid(rend_data->desc_id_fetch, query,
                                  hsdirs);
  } else {
    /* Query data is invalid. */
    ret = -1;
    goto error;
  }

 error:
  return ret;
}

/** Unless we already have a descriptor for <b>rend_query</b> with at least
 * one (possibly) working introduction point in it, start a connection to a
 * hidden service directory to fetch a v2 rendezvous service descriptor. */
void
rend_client_refetch_v2_renddesc(rend_data_t *rend_query)
{
  rend_cache_entry_t *e = NULL;
  const char *onion_address = rend_data_get_address(rend_query);

  tor_assert(rend_query);
  /* Before fetching, check if we already have a usable descriptor here. */
  if (rend_cache_lookup_entry(onion_address, -1, &e) == 0 &&
      rend_client_any_intro_points_usable(e)) {
    log_info(LD_REND, "We would fetch a v2 rendezvous descriptor, but we "
                      "already have a usable descriptor here. Not fetching.");
    return;
  }
  /* Are we configured to fetch descriptors? */
  if (!get_options()->FetchHidServDescriptors) {
    log_warn(LD_REND, "We received an onion address for a v2 rendezvous "
        "service descriptor, but are not fetching service descriptors.");
    return;
  }
  log_debug(LD_REND, "Fetching v2 rendezvous descriptor for service %s",
            safe_str_client(onion_address));

  rend_client_fetch_v2_desc(rend_query, NULL);
  /* We don't need to look the error code because either on failure or
   * success, the necessary steps to continue the HS connection will be
   * triggered once the descriptor arrives or if all fetch failed. */
  return;
}

/** Cancel all rendezvous descriptor fetches currently in progress.
 */
void
rend_client_cancel_descriptor_fetches(void)
{
  smartlist_t *connection_array = get_connection_array();

  SMARTLIST_FOREACH_BEGIN(connection_array, connection_t *, conn) {
    if (conn->type == CONN_TYPE_DIR &&
        conn->purpose == DIR_PURPOSE_FETCH_RENDDESC_V2) {
      /* It's a rendezvous descriptor fetch in progress -- cancel it
       * by marking the connection for close.
       *
       * Even if this connection has already reached EOF, this is
       * enough to make sure that if the descriptor hasn't been
       * processed yet, it won't be.  See the end of
       * connection_handle_read; connection_reached_eof (indirectly)
       * processes whatever response the connection received. */

      const rend_data_t *rd = (TO_DIR_CONN(conn))->rend_data;
      if (!rd) {
        log_warn(LD_BUG | LD_REND,
                 "Marking for close dir conn fetching rendezvous "
                 "descriptor for unknown service!");
      } else {
        log_debug(LD_REND, "Marking for close dir conn fetching "
                  "rendezvous descriptor for service %s",
                  safe_str(rend_data_get_address(rd)));
      }
      connection_mark_for_close(conn);
    }
  } SMARTLIST_FOREACH_END(conn);
}

/** Mark <b>failed_intro</b> as a failed introduction point for the
 * hidden service specified by <b>rend_query</b>. If the HS now has no
 * usable intro points, or we do not have an HS descriptor for it,
 * then launch a new renddesc fetch.
 *
 * If <b>failure_type</b> is INTRO_POINT_FAILURE_GENERIC, remove the
 * intro point from (our parsed copy of) the HS descriptor.
 *
 * If <b>failure_type</b> is INTRO_POINT_FAILURE_TIMEOUT, mark the
 * intro point as 'timed out'; it will not be retried until the
 * current hidden service connection attempt has ended or it has
 * appeared in a newly fetched rendezvous descriptor.
 *
 * If <b>failure_type</b> is INTRO_POINT_FAILURE_UNREACHABLE,
 * increment the intro point's reachability-failure count; if it has
 * now failed MAX_INTRO_POINT_REACHABILITY_FAILURES or more times,
 * remove the intro point from (our parsed copy of) the HS descriptor.
 *
 * Return -1 if error, 0 if no usable intro points remain or service
 * unrecognized, 1 if recognized and some intro points remain.
 */
int
rend_client_report_intro_point_failure(extend_info_t *failed_intro,
                                       rend_data_t *rend_data,
                                       unsigned int failure_type)
{
  int i, r;
  rend_cache_entry_t *ent;
  connection_t *conn;
  const char *onion_address = rend_data_get_address(rend_data);

  r = rend_cache_lookup_entry(onion_address, -1, &ent);
  if (r < 0) {
    /* Either invalid onion address or cache entry not found. */
    switch (-r) {
    case EINVAL:
      log_warn(LD_BUG, "Malformed service ID %s.",
               escaped_safe_str_client(onion_address));
      return -1;
    case ENOENT:
      log_info(LD_REND, "Unknown service %s. Re-fetching descriptor.",
               escaped_safe_str_client(onion_address));
      rend_client_refetch_v2_renddesc(rend_data);
      return 0;
    default:
      log_warn(LD_BUG, "Unknown cache lookup returned code: %d", r);
      return -1;
    }
  }
  /* The intro points are not checked here if they are usable or not because
   * this is called when an intro point circuit is closed thus there must be
   * at least one intro point that is usable and is about to be flagged. */

  for (i = 0; i < smartlist_len(ent->parsed->intro_nodes); i++) {
    rend_intro_point_t *intro = smartlist_get(ent->parsed->intro_nodes, i);
    if (tor_memeq(failed_intro->identity_digest,
                intro->extend_info->identity_digest, DIGEST_LEN)) {
      switch (failure_type) {
      default:
        log_warn(LD_BUG, "Unknown failure type %u. Removing intro point.",
                 failure_type);
        tor_fragile_assert();
        FALLTHROUGH_UNLESS_ALL_BUGS_ARE_FATAL;
      case INTRO_POINT_FAILURE_GENERIC:
        rend_cache_intro_failure_note(failure_type,
                                      (uint8_t *)failed_intro->identity_digest,
                                      onion_address);
        rend_intro_point_free(intro);
        smartlist_del(ent->parsed->intro_nodes, i);
        break;
      case INTRO_POINT_FAILURE_TIMEOUT:
        intro->timed_out = 1;
        break;
      case INTRO_POINT_FAILURE_UNREACHABLE:
        ++(intro->unreachable_count);
        {
          int zap_intro_point =
            intro->unreachable_count >= MAX_INTRO_POINT_REACHABILITY_FAILURES;
          log_info(LD_REND, "Failed to reach this intro point %u times.%s",
                   intro->unreachable_count,
                   zap_intro_point ? " Removing from descriptor.": "");
          if (zap_intro_point) {
            rend_cache_intro_failure_note(
                failure_type,
                (uint8_t *) failed_intro->identity_digest, onion_address);
            rend_intro_point_free(intro);
            smartlist_del(ent->parsed->intro_nodes, i);
          }
        }
        break;
      }
      break;
    }
  }

  if (! rend_client_any_intro_points_usable(ent)) {
    log_info(LD_REND,
             "No more intro points remain for %s. Re-fetching descriptor.",
             escaped_safe_str_client(onion_address));
    rend_client_refetch_v2_renddesc(rend_data);

    /* move all pending streams back to renddesc_wait */
    /* NOTE: We can now do this faster, if we use pending_entry_connections */
    while ((conn = connection_get_by_type_state_rendquery(CONN_TYPE_AP,
                                   AP_CONN_STATE_CIRCUIT_WAIT,
                                   onion_address))) {
      connection_ap_mark_as_waiting_for_renddesc(TO_ENTRY_CONN(conn));
    }

    return 0;
  }
  log_info(LD_REND,"%d options left for %s.",
           smartlist_len(ent->parsed->intro_nodes),
           escaped_safe_str_client(onion_address));
  return 1;
}

/** The service sent us a rendezvous cell; join the circuits. */
int
rend_client_receive_rendezvous(origin_circuit_t *circ, const uint8_t *request,
                               size_t request_len)
{
  if (request_len != DH1024_KEY_LEN+DIGEST_LEN) {
    log_warn(LD_PROTOCOL,"Incorrect length (%d) on RENDEZVOUS2 cell.",
             (int)request_len);
    goto err;
  }

  if (hs_circuit_setup_e2e_rend_circ_legacy_client(circ, request) < 0) {
    log_warn(LD_GENERAL, "Failed to setup circ");
    goto err;
  }
  return 0;

 err:
  circuit_mark_for_close(TO_CIRCUIT(circ), END_CIRC_REASON_TORPROTOCOL);
  return -1;
}

/** Find all the apconns in state AP_CONN_STATE_RENDDESC_WAIT that are
 * waiting on <b>query</b>. If there's a working cache entry here with at
 * least one intro point, move them to the next state. */
void
rend_client_desc_trynow(const char *query)
{
  entry_connection_t *conn;
  rend_cache_entry_t *entry;
  const rend_data_t *rend_data;
  time_t now = time(NULL);

  smartlist_t *conns = get_connection_array();
  SMARTLIST_FOREACH_BEGIN(conns, connection_t *, base_conn) {
    if (base_conn->type != CONN_TYPE_AP ||
        base_conn->state != AP_CONN_STATE_RENDDESC_WAIT ||
        base_conn->marked_for_close)
      continue;
    conn = TO_ENTRY_CONN(base_conn);
    rend_data = ENTRY_TO_EDGE_CONN(conn)->rend_data;
    if (!rend_data)
      continue;
    const char *onion_address = rend_data_get_address(rend_data);
    if (rend_cmp_service_ids(query, onion_address))
      continue;
    assert_connection_ok(base_conn, now);
    if (rend_cache_lookup_entry(onion_address, -1,
                                &entry) == 0 &&
        rend_client_any_intro_points_usable(entry)) {
      /* either this fetch worked, or it failed but there was a
       * valid entry from before which we should reuse */
      log_info(LD_REND,"Rend desc is usable. Launching circuits.");
      base_conn->state = AP_CONN_STATE_CIRCUIT_WAIT;

      /* restart their timeout values, so they get a fair shake at
       * connecting to the hidden service. */
      base_conn->timestamp_created = now;
      base_conn->timestamp_last_read_allowed = now;
      base_conn->timestamp_last_write_allowed = now;

      connection_ap_mark_as_pending_circuit(conn);
    } else { /* 404, or fetch didn't get that far */
      log_notice(LD_REND,"Closing stream for '%s.onion': hidden service is "
                 "unavailable (try again later).",
                 safe_str_client(query));
      connection_mark_unattached_ap(conn, END_STREAM_REASON_RESOLVEFAILED);
      rend_client_note_connection_attempt_ended(rend_data);
    }
  } SMARTLIST_FOREACH_END(base_conn);
}

/** Clear temporary state used only during an attempt to connect to the
 * hidden service with <b>rend_data</b>. Called when a connection attempt
 * has ended; it is possible for this to be called multiple times while
 * handling an ended connection attempt, and any future changes to this
 * function must ensure it remains idempotent. */
void
rend_client_note_connection_attempt_ended(const rend_data_t *rend_data)
{
  unsigned int have_onion = 0;
  rend_cache_entry_t *cache_entry = NULL;
  const char *onion_address = rend_data_get_address(rend_data);
  rend_data_v2_t *rend_data_v2 = TO_REND_DATA_V2(rend_data);

  if (onion_address[0] != '\0') {
    /* Ignore return value; we find an entry, or we don't. */
    (void) rend_cache_lookup_entry(onion_address, -1, &cache_entry);
    have_onion = 1;
  }

  /* Clear the timed_out flag on all remaining intro points for this HS. */
  if (cache_entry != NULL) {
    SMARTLIST_FOREACH(cache_entry->parsed->intro_nodes,
                      rend_intro_point_t *, ip,
                      ip->timed_out = 0; );
  }

  /* Remove the HS's entries in last_hid_serv_requests. */
  if (have_onion) {
    unsigned int replica;
    for (replica = 0; replica < ARRAY_LENGTH(rend_data_v2->descriptor_id);
         replica++) {
      const char *desc_id = rend_data_v2->descriptor_id[replica];
      purge_v2_hidserv_req(desc_id);
    }
    log_info(LD_REND, "Connection attempt for %s has ended; "
             "cleaning up temporary state.",
             safe_str_client(onion_address));
  } else {
    /* We only have an ID for a fetch. Probably used by HSFETCH. */
    purge_v2_hidserv_req(rend_data_v2->desc_id_fetch);
  }
}

/** Return a newly allocated extend_info_t* for a randomly chosen introduction
 * point for the named hidden service.  Return NULL if all introduction points
 * have been tried and failed.
 */
extend_info_t *
rend_client_get_random_intro(const rend_data_t *rend_query)
{
  int ret;
  extend_info_t *result;
  rend_cache_entry_t *entry;
  const char *onion_address = rend_data_get_address(rend_query);

  ret = rend_cache_lookup_entry(onion_address, -1, &entry);
  if (ret < 0 || !rend_client_any_intro_points_usable(entry)) {
    log_warn(LD_REND,
             "Query '%s' didn't have valid rend desc in cache. Failing.",
             safe_str_client(onion_address));
    /* XXX: Should we refetch the descriptor here if the IPs are not usable
     * anymore ?. */
    return NULL;
  }

  /* See if we can get a node that complies with ExcludeNodes */
  if ((result = rend_client_get_random_intro_impl(entry, 1, 1)))
    return result;
  /* If not, and StrictNodes is not set, see if we can return any old node
   */
  if (!get_options()->StrictNodes)
    return rend_client_get_random_intro_impl(entry, 0, 1);
  return NULL;
}

/** As rend_client_get_random_intro, except assume that StrictNodes is set
 * iff <b>strict</b> is true. If <b>warnings</b> is false, don't complain
 * to the user when we're out of nodes, even if StrictNodes is true.
 */
static extend_info_t *
rend_client_get_random_intro_impl(const rend_cache_entry_t *entry,
                                  const int strict,
                                  const int warnings)
{
  int i;

  rend_intro_point_t *intro;
  const or_options_t *options = get_options();
  smartlist_t *usable_nodes;
  int n_excluded = 0;
  char service_id[REND_SERVICE_ID_LEN_BASE32 + 1];

  /* We'll keep a separate list of the usable nodes.  If this becomes empty,
   * no nodes are usable.  */
  usable_nodes = smartlist_new();
  smartlist_add_all(usable_nodes, entry->parsed->intro_nodes);

  /* Get service ID so we can use it to query the failure cache. If we fail to
   * parse it, this cache entry is no good. */
  if (BUG(rend_get_service_id(entry->parsed->pk, service_id) < 0)) {
    smartlist_free(usable_nodes);
    return NULL;
  }

  /* Remove the intro points that have timed out during this HS
   * connection attempt from our list of usable nodes. */
  SMARTLIST_FOREACH_BEGIN(usable_nodes, const rend_intro_point_t *, ip) {
    bool failed_intro =
      rend_cache_intro_failure_exists(service_id,
                       (const uint8_t *) ip->extend_info->identity_digest);
    if (ip->timed_out || failed_intro) {
      SMARTLIST_DEL_CURRENT(usable_nodes, ip);
    };
  } SMARTLIST_FOREACH_END(ip);

 again:
  if (smartlist_len(usable_nodes) == 0) {
    if (n_excluded && get_options()->StrictNodes && warnings) {
      /* We only want to warn if StrictNodes is really set. Otherwise
       * we're just about to retry anyways.
       */
      log_warn(LD_REND, "All introduction points for hidden service are "
               "at excluded relays, and StrictNodes is set. Skipping.");
    }
    smartlist_free(usable_nodes);
    return NULL;
  }

  i = crypto_rand_int(smartlist_len(usable_nodes));
  intro = smartlist_get(usable_nodes, i);
  if (BUG(!intro->extend_info)) {
    /* This should never happen, but it isn't fatal, just try another */
    smartlist_del(usable_nodes, i);
    goto again;
  }
  /* All version 2 HS descriptors come with a TAP onion key.
   * Clients used to try to get the TAP onion key from the consensus, but this
   * meant that hidden services could discover which consensus clients have. */
  if (!extend_info_supports_tap(intro->extend_info)) {
    log_info(LD_REND, "The HS descriptor is missing a TAP onion key for the "
             "intro-point relay '%s'; trying another.",
             safe_str_client(extend_info_describe(intro->extend_info)));
    smartlist_del(usable_nodes, i);
    goto again;
  }
  /* Check if we should refuse to talk to this router. */
  if (strict &&
      routerset_contains_extendinfo(options->ExcludeNodes,
                                    intro->extend_info)) {
    n_excluded++;
    smartlist_del(usable_nodes, i);
    goto again;
  }

  smartlist_free(usable_nodes);
  return extend_info_dup(intro->extend_info);
}

/** Return true iff any introduction points still listed in <b>entry</b> are
 * usable. */
int
rend_client_any_intro_points_usable(const rend_cache_entry_t *entry)
{
  extend_info_t *extend_info =
    rend_client_get_random_intro_impl(entry, get_options()->StrictNodes, 0);

  int rv = (extend_info != NULL);

  extend_info_free(extend_info);
  return rv;
}

/** Client-side authorizations for hidden services; map of onion address to
 * rend_service_authorization_t*. */
static strmap_t *auth_hid_servs = NULL;

/** Look up the client-side authorization for the hidden service with
 * <b>onion_address</b>. Return NULL if no authorization is available for
 * that address. */
rend_service_authorization_t*
rend_client_lookup_service_authorization(const char *onion_address)
{
  tor_assert(onion_address);
  if (!auth_hid_servs) return NULL;
  return strmap_get(auth_hid_servs, onion_address);
}

#define rend_service_authorization_free(val)                    \
  FREE_AND_NULL(rend_service_authorization_t,                   \
                rend_service_authorization_free_, (val))

/** Helper: Free storage held by rend_service_authorization_t. */
static void
rend_service_authorization_free_(rend_service_authorization_t *auth)
{
  tor_free(auth);
}

/** Helper for strmap_free. */
static void
rend_service_authorization_free_void(void *service_auth)
{
  rend_service_authorization_free_(service_auth);
}

/** Release all the storage held in auth_hid_servs.
 */
void
rend_service_authorization_free_all(void)
{
  if (!auth_hid_servs) {
    return;
  }
  strmap_free(auth_hid_servs, rend_service_authorization_free_void);
  auth_hid_servs = NULL;
}

/** Parse <b>config_line</b> as a client-side authorization for a hidden
 * service and add it to the local map of hidden service authorizations.
 * Return 0 for success and -1 for failure. */
int
rend_parse_service_authorization(const or_options_t *options,
                                 int validate_only)
{
  config_line_t *line;
  int res = -1;
  strmap_t *parsed = strmap_new();
  smartlist_t *sl = smartlist_new();
  rend_service_authorization_t *auth = NULL;
  char *err_msg = NULL;

  for (line = options->HidServAuth; line; line = line->next) {
    char *onion_address, *descriptor_cookie;
    auth = NULL;
    SMARTLIST_FOREACH(sl, char *, c, tor_free(c););
    smartlist_clear(sl);
    smartlist_split_string(sl, line->value, " ",
                           SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, 3);
    if (smartlist_len(sl) < 2) {
      log_warn(LD_CONFIG, "Configuration line does not consist of "
               "\"onion-address authorization-cookie [service-name]\": "
               "'%s'", line->value);
      goto err;
    }
    auth = tor_malloc_zero(sizeof(rend_service_authorization_t));
    /* Parse onion address. */
    onion_address = smartlist_get(sl, 0);
    if (strlen(onion_address) != REND_SERVICE_ADDRESS_LEN ||
        strcmpend(onion_address, ".onion")) {
      log_warn(LD_CONFIG, "Onion address has wrong format: '%s'",
               onion_address);
      goto err;
    }
    strlcpy(auth->onion_address, onion_address, REND_SERVICE_ID_LEN_BASE32+1);
    if (!rend_valid_v2_service_id(auth->onion_address)) {
      log_warn(LD_CONFIG, "Onion address has wrong format: '%s'",
               onion_address);
      goto err;
    }
    /* Parse descriptor cookie. */
    descriptor_cookie = smartlist_get(sl, 1);
    if (rend_auth_decode_cookie(descriptor_cookie, auth->descriptor_cookie,
                                &auth->auth_type, &err_msg) < 0) {
      tor_assert(err_msg);
      log_warn(LD_CONFIG, "%s", err_msg);
      tor_free(err_msg);
      goto err;
    }
    if (strmap_get(parsed, auth->onion_address)) {
      log_warn(LD_CONFIG, "Duplicate authorization for the same hidden "
                          "service.");
      goto err;
    }
    strmap_set(parsed, auth->onion_address, auth);
    auth = NULL;
  }
  res = 0;
  goto done;
 err:
  res = -1;
 done:
  rend_service_authorization_free(auth);
  SMARTLIST_FOREACH(sl, char *, c, tor_free(c););
  smartlist_free(sl);
  if (!validate_only && res == 0) {
    rend_service_authorization_free_all();
    auth_hid_servs = parsed;
  } else {
    strmap_free(parsed, rend_service_authorization_free_void);
  }
  return res;
}

/** The given circuit is being freed. Take appropriate action if it is of
 * interest to the client subsystem. */
void
rend_client_circuit_cleanup_on_free(const circuit_t *circ)
{
  int reason, orig_reason;
  bool has_timed_out, ip_is_redundant;
  const origin_circuit_t *ocirc = NULL;

  tor_assert(circ);
  tor_assert(CIRCUIT_IS_ORIGIN(circ));

  reason = circ->marked_for_close_reason;
  orig_reason = circ->marked_for_close_orig_reason;
  ocirc = CONST_TO_ORIGIN_CIRCUIT(circ);
  tor_assert(ocirc->rend_data);

  has_timed_out = (reason == END_CIRC_REASON_TIMEOUT);
  ip_is_redundant = (orig_reason == END_CIRC_REASON_IP_NOW_REDUNDANT);

  switch (circ->purpose) {
  case CIRCUIT_PURPOSE_C_INTRODUCE_ACK_WAIT:
  {
    if (ip_is_redundant) {
      break;
    }
    tor_assert(circ->state == CIRCUIT_STATE_OPEN);
    tor_assert(ocirc->build_state->chosen_exit);
    /* Treat this like getting a nack from it */
    log_info(LD_REND, "Failed intro circ %s to %s (awaiting ack). %s",
        safe_str_client(rend_data_get_address(ocirc->rend_data)),
        safe_str_client(build_state_get_exit_nickname(ocirc->build_state)),
        has_timed_out ? "Recording timeout." : "Removing from descriptor.");
    rend_client_report_intro_point_failure(ocirc->build_state->chosen_exit,
                                           ocirc->rend_data,
                                           has_timed_out ?
                                           INTRO_POINT_FAILURE_TIMEOUT :
                                           INTRO_POINT_FAILURE_GENERIC);
    break;
  }
  case CIRCUIT_PURPOSE_C_INTRODUCING:
  {
    /* Ignore if we were introducing and it timed out, we didn't pick an exit
     * point yet (IP) or the reason indicate that it was a redundant IP. */
    if (has_timed_out || !ocirc->build_state->chosen_exit || ip_is_redundant) {
      break;
    }
    log_info(LD_REND, "Failed intro circ %s to %s "
             "(building circuit to intro point). "
             "Marking intro point as possibly unreachable.",
             safe_str_client(rend_data_get_address(ocirc->rend_data)),
             safe_str_client(build_state_get_exit_nickname(
                                                  ocirc->build_state)));
    rend_client_report_intro_point_failure(ocirc->build_state->chosen_exit,
                                           ocirc->rend_data,
                                           INTRO_POINT_FAILURE_UNREACHABLE);
    break;
  }
  default:
    break;
  }
}
