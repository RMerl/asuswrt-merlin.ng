/* Copyright (c) 2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file metrics_store_entry.c
 * @brief Metrics store entry which contains the gathered data.
 **/

#define METRICS_STORE_ENTRY_PRIVATE

#include <string.h>

#include "orconfig.h"

#include "lib/container/smartlist.h"
#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"

#include "lib/metrics/metrics_store_entry.h"

/*
 * Public API.
 */

/** Return newly allocated store entry of type COUNTER. */
metrics_store_entry_t *
metrics_store_entry_new(const metrics_type_t type, const char *name,
                        const char *help)
{
  metrics_store_entry_t *entry = tor_malloc_zero(sizeof(*entry));

  tor_assert(name);

  entry->type = type;
  entry->name = tor_strdup(name);
  entry->labels = smartlist_new();
  if (help) {
    entry->help = tor_strdup(help);
  }

  return entry;
}

/** Free a store entry. */
void
metrics_store_entry_free_(metrics_store_entry_t *entry)
{
  if (!entry) {
    return;
  }
  SMARTLIST_FOREACH(entry->labels, char *, l, tor_free(l));
  smartlist_free(entry->labels);
  tor_free(entry->name);
  tor_free(entry->help);
  tor_free(entry);
}

/** Update a store entry with value. */
void
metrics_store_entry_update(metrics_store_entry_t *entry, const int64_t value)
{
  tor_assert(entry);

  switch (entry->type) {
  case METRICS_TYPE_COUNTER:
    /* Counter can ONLY be positive. */
    if (BUG(value < 0)) {
      return;
    }
    entry->u.counter.value += value;
    break;
  case METRICS_TYPE_GAUGE:
    /* Gauge can increment or decrement. And can be positive or negative. */
    entry->u.gauge.value += value;
    break;
  }
}

/** Reset a store entry that is set its metric data to 0. */
void
metrics_store_entry_reset(metrics_store_entry_t *entry)
{
  tor_assert(entry);
  /* Everything back to 0. */
  memset(&entry->u, 0, sizeof(entry->u));
}

/** Return store entry value. */
int64_t
metrics_store_entry_get_value(const metrics_store_entry_t *entry)
{
  tor_assert(entry);

  switch (entry->type) {
  case METRICS_TYPE_COUNTER:
    if (entry->u.counter.value > INT64_MAX) {
      return INT64_MAX;
    }
    return entry->u.counter.value;
  case METRICS_TYPE_GAUGE:
    return entry->u.gauge.value;
  }

  // LCOV_EXCL_START
  tor_assert_unreached();
  // LCOV_EXCL_STOP
}

/** Add a label into the given entry.*/
void
metrics_store_entry_add_label(metrics_store_entry_t *entry,
                              const char *label)
{
  tor_assert(entry);
  tor_assert(label);

  smartlist_add(entry->labels, tor_strdup(label));
}

/** Return true iff the given entry has the given label. */
bool
metrics_store_entry_has_label(const metrics_store_entry_t *entry,
                              const char *label)
{
  tor_assert(entry);
  tor_assert(label);

  return smartlist_contains_string(entry->labels, label);
}
