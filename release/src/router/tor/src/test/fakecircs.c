/* Copyright (c) 2019-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file fakecircs.c
 * \brief Fake circuits API for unit test.
 **/

#define CIRCUITBUILD_PRIVATE
#define CIRCUITLIST_PRIVATE
#define CRYPT_PATH_PRIVATE

#include "core/or/or.h"

#include "core/crypto/relay_crypto.h"
#include "core/or/channel.h"
#include "core/or/circuitbuild.h"
#include "core/or/circuitlist.h"
#include "core/or/circuitpadding.h"
#include "core/or/crypt_path.h"
#include "core/or/relay.h"
#include "core/or/relay_crypto_st.h"

#include "test/fakecircs.h"

/** Return newly allocated OR circuit using the given nchan and pchan. It must
 * be freed with the free_fake_orcirc(). */
or_circuit_t *
new_fake_orcirc(channel_t *nchan, channel_t *pchan)
{
  or_circuit_t *orcirc = NULL;
  circuit_t *circ = NULL;
  crypt_path_t tmp_cpath;
  char whatevs_key[CPATH_KEY_MATERIAL_LEN];

  orcirc = tor_malloc_zero(sizeof(*orcirc));
  circ = &(orcirc->base_);
  circ->magic = OR_CIRCUIT_MAGIC;

  circuit_set_n_circid_chan(circ, get_unique_circ_id_by_chan(nchan), nchan);
  cell_queue_init(&(circ->n_chan_cells));

  circ->n_hop = NULL;
  circ->streams_blocked_on_n_chan = 0;
  circ->streams_blocked_on_p_chan = 0;
  circ->n_delete_pending = 0;
  circ->p_delete_pending = 0;
  circ->received_destroy = 0;
  circ->state = CIRCUIT_STATE_OPEN;
  circ->purpose = CIRCUIT_PURPOSE_OR;
  circ->package_window = CIRCWINDOW_START_MAX;
  circ->deliver_window = CIRCWINDOW_START_MAX;
  circ->n_chan_create_cell = NULL;

  circuit_set_p_circid_chan(orcirc, get_unique_circ_id_by_chan(pchan), pchan);
  cell_queue_init(&(orcirc->p_chan_cells));

  memset(&tmp_cpath, 0, sizeof(tmp_cpath));
  if (cpath_init_circuit_crypto(&tmp_cpath, whatevs_key,
                                sizeof(whatevs_key), 0, 0)<0) {
    log_warn(LD_BUG,"Circuit initialization failed");
    return NULL;
  }
  orcirc->crypto = tmp_cpath.pvt_crypto;

  return orcirc;
}

/** Free fake OR circuit which MUST be created by new_fake_orcirc(). */
void
free_fake_orcirc(or_circuit_t *orcirc)
{
  if (!orcirc) {
    return;
  }

  circuit_t *circ = TO_CIRCUIT(orcirc);

  relay_crypto_clear(&orcirc->crypto);

  circpad_circuit_free_all_machineinfos(circ);

  if (orcirc->p_chan && orcirc->p_chan->cmux) {
    circuitmux_detach_circuit(orcirc->p_chan->cmux, circ);
  }
  if (circ->n_chan && circ->n_chan->cmux) {
    circuitmux_detach_circuit(circ->n_chan->cmux, circ);
  }

  tor_free_(circ);
}
