/* Copyright (c) 2020-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file hs_metrics.h
 * @brief Header for feature/hs/hs_metrics.c
 **/

#ifndef TOR_FEATURE_HS_HS_METRICS_H
#define TOR_FEATURE_HS_HS_METRICS_H

#include "lib/container/smartlist.h"
#include "lib/crypt_ops/crypto_ed25519.h"

#define HS_METRICS_ENTRY_PRIVATE
#include "feature/hs/hs_metrics_entry.h"
#include "feature/hs/hs_service.h"

/* Init and Free. */
void hs_metrics_service_init(hs_service_t *service);
void hs_metrics_service_free(hs_service_t *service);

/* Accessors. */
const smartlist_t *hs_metrics_get_stores(void);

/* Metrics Update. */
void hs_metrics_update_by_ident(const hs_metrics_key_t key,
                                const ed25519_public_key_t *ident_pk,
                                const uint16_t port, int64_t n);
void hs_metrics_update_by_service(const hs_metrics_key_t key,
                                  hs_service_t *service, const uint16_t port,
                                  int64_t n);

/** New introducion request received. */
#define hs_metrics_new_introduction(s) \
  hs_metrics_update_by_service(HS_METRICS_NUM_INTRODUCTIONS, (s), 0, 1)

/** Number of bytes written to the application from the service. */
#define hs_metrics_app_write_bytes(i, port, n) \
  hs_metrics_update_by_ident(HS_METRICS_APP_WRITE_BYTES, (i), (port), (n))

/** Number of bytes read from the application to the service. */
#define hs_metrics_app_read_bytes(i, port, n) \
  hs_metrics_update_by_ident(HS_METRICS_APP_READ_BYTES, (i), (port), (n))

/** Newly established rendezvous. This is called as soon as the circuit purpose
 * is REND_JOINED which is when the RENDEZVOUS2 cell is sent. */
#define hs_metrics_new_established_rdv(s) \
  hs_metrics_update_by_service(HS_METRICS_NUM_ESTABLISHED_RDV, (s), 0, 1)

/** Established rendezvous closed. This is called when the circuit in
 * REND_JOINED state is marked for close. */
#define hs_metrics_close_established_rdv(i) \
  hs_metrics_update_by_ident(HS_METRICS_NUM_ESTABLISHED_RDV, (i), 0, -1)

/** New rendezvous circuit being launched. */
#define hs_metrics_new_rdv(i) \
  hs_metrics_update_by_ident(HS_METRICS_NUM_RDV, (i), 0, 1)

/** New introduction circuit has been established. This is called when the
 * INTRO_ESTABLISHED has been received by the service. */
#define hs_metrics_new_established_intro(s) \
  hs_metrics_update_by_service(HS_METRICS_NUM_ESTABLISHED_INTRO, (s), 0, 1)

/** Established introduction circuit closes. This is called when
 * INTRO_ESTABLISHED circuit is marked for close. */
#define hs_metrics_close_established_intro(i) \
  hs_metrics_update_by_ident(HS_METRICS_NUM_ESTABLISHED_INTRO, (i), 0, -1)

#endif /* !defined(TOR_FEATURE_HS_HS_METRICS_H) */
