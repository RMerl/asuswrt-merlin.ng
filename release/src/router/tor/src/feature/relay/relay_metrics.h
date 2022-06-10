/* Copyright (c) 2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file relay_metrics.h
 * @brief Header for feature/relay/relay_metrics.c
 **/

#ifndef TOR_FEATURE_RELAY_RELAY_METRICS_H
#define TOR_FEATURE_RELAY_RELAY_METRICS_H

#include "lib/container/smartlist.h"
#include "lib/metrics/metrics_common.h"

/** Metrics key for each reported metrics. This key is also used as an index in
 * the base_metrics array. */
typedef enum {
  /** Number of OOM invocation. */
  RELAY_METRICS_NUM_OOM_BYTES  = 0,
  /** Number of onionskines handled. */
  RELAY_METRICS_NUM_ONIONSKINS = 1,
  /** Number of sockets. */
  RELAY_METRICS_NUM_SOCKETS    = 2,
  /** Number of global connection rate limit. */
  RELAY_METRICS_NUM_GLOBAL_RW_LIMIT = 3,
  /** Number of DNS queries. */
  RELAY_METRICS_NUM_DNS        = 4,
  /** Number of DNS query errors. */
  RELAY_METRICS_NUM_DNS_ERRORS = 5,
  /** Number of TCP exhaustion reached. */
  RELAY_METRICS_NUM_TCP_EXHAUSTION = 6,
} relay_metrics_key_t;

/** The metadata of a relay metric. */
typedef struct relay_metrics_entry_t {
  /* Metric key used as a static array index. */
  relay_metrics_key_t key;
  /* Metric type. */
  metrics_type_t type;
  /* Metrics output name. */
  const char *name;
  /* Metrics output help comment. */
  const char *help;
  /* Update value function. */
  void (*fill_fn)(void);
} relay_metrics_entry_t;

/* Init. */
void relay_metrics_init(void);
void relay_metrics_free(void);

/* Accessors. */
const smartlist_t *relay_metrics_get_stores(void);

#endif /* !defined(TOR_FEATURE_RELAY_RELAY_METRICS_H) */
