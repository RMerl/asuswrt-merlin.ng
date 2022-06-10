/* Copyright (c) 2020-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file hs_metrics_entry.h
 * @brief Header for feature/hs/hs_metrics_entry.c
 **/

#ifndef TOR_FEATURE_HS_METRICS_ENTRY_H
#define TOR_FEATURE_HS_METRICS_ENTRY_H

#ifdef HS_METRICS_ENTRY_PRIVATE

#include "lib/metrics/metrics_common.h"

/** Metrics key which are used as an index in the main base metrics array. */
typedef enum {
  /** Number of introduction requests. */
  HS_METRICS_NUM_INTRODUCTIONS = 0,
  /** Number of bytes written from onion service to application. */
  HS_METRICS_APP_WRITE_BYTES = 1,
  /** Number of bytes read from application to onion service. */
  HS_METRICS_APP_READ_BYTES = 2,
  /** Number of established rendezsvous. */
  HS_METRICS_NUM_ESTABLISHED_RDV = 3,
  /** Number of rendezsvous circuits created. */
  HS_METRICS_NUM_RDV = 4,
  /** Number of established introducton points. */
  HS_METRICS_NUM_ESTABLISHED_INTRO = 5,
} hs_metrics_key_t;

/** The metadata of an HS metrics. */
typedef struct hs_metrics_entry_t {
  /* Metric key used as a static array index. */
  hs_metrics_key_t key;
  /* Metric type. */
  metrics_type_t type;
  /* Metrics output name. */
  const char *name;
  /* Metrics output help comment. */
  const char *help;
  /* True iff a port label should be added to the metrics entry. */
  bool port_as_label;
} hs_metrics_entry_t;

extern const hs_metrics_entry_t base_metrics[];
extern const size_t base_metrics_size;

#endif /* defined(HS_METRICS_ENTRY_PRIVATE) */

#endif /* !defined(TOR_FEATURE_HS_METRICS_ENTRY_H) */
