/* Copyright (c) 2020-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file prometheus.c
 * @brief Metrics format driver for Prometheus data model.
 **/

#define METRICS_STORE_ENTRY_PRIVATE

#include "orconfig.h"

#include "lib/container/smartlist.h"
#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"
#include "lib/string/printf.h"

#include "lib/metrics/prometheus.h"

#include <string.h>

/** Return a static buffer containing all the labels properly formatted
 * for the output as a string.
 *
 * Subsequent calls to this invalidates the previous result. */
static const char *
format_labels(smartlist_t *labels)
{
  static char buf[1024];
  char *line = NULL;

  if (smartlist_len(labels) == 0) {
    buf[0] = '\0';
    goto end;
  }

  line = smartlist_join_strings(labels, ",", 0, NULL);
  tor_snprintf(buf, sizeof(buf), "%s", line);

 end:
  tor_free(line);
  return buf;
}

/** Write the string representation of the histogram entry to the specified
 * buffer.
 *
 * Note: entry **must** be a histogram.
 */
static void
format_histogram(const metrics_store_entry_t *entry, buf_t *data)
{
  tor_assert(entry->type == METRICS_TYPE_HISTOGRAM);

  const char *labels = format_labels(entry->labels);

  for (size_t i = 0; i < entry->u.histogram.bucket_count; ++i) {
    metrics_histogram_bucket_t hb = entry->u.histogram.buckets[i];
    if (strlen(labels) > 0) {
      buf_add_printf(data, "%s_bucket{%s,le=\"%.2f\"} %" PRIi64 "\n",
                     entry->name, labels, (double)hb.bucket, hb.value);
    } else {
      buf_add_printf(data, "%s_bucket{le=\"%.2f\"} %" PRIi64 "\n",
                     entry->name, (double)hb.bucket, hb.value);
    }
  }

  if (strlen(labels) > 0) {
    buf_add_printf(data, "%s_bucket{%s,le=\"+Inf\"} %" PRIi64 "\n",
                   entry->name, labels,
                   metrics_store_hist_entry_get_count(entry));
    buf_add_printf(data, "%s_sum{%s} %" PRIi64 "\n", entry->name, labels,
                   metrics_store_hist_entry_get_sum(entry));
    buf_add_printf(data, "%s_count{%s} %" PRIi64 "\n", entry->name, labels,
                   metrics_store_hist_entry_get_count(entry));
  } else {
    buf_add_printf(data, "%s_bucket{le=\"+Inf\"} %" PRIi64 "\n", entry->name,
                   metrics_store_hist_entry_get_count(entry));
    buf_add_printf(data, "%s_sum %" PRIi64 "\n", entry->name,
                   metrics_store_hist_entry_get_sum(entry));
    buf_add_printf(data, "%s_count %" PRIi64 "\n", entry->name,
                   metrics_store_hist_entry_get_count(entry));
  }
}

/** Format the given entry in to the buffer data. */
void
prometheus_format_store_entry(const metrics_store_entry_t *entry, buf_t *data,
                              bool no_comment)
{
  tor_assert(entry);
  tor_assert(data);

  if (!no_comment) {
    buf_add_printf(data, "# HELP %s %s\n", entry->name, entry->help);
    buf_add_printf(data, "# TYPE %s %s\n", entry->name,
                   metrics_type_to_str(entry->type));
  }

  switch (entry->type) {
  case METRICS_TYPE_COUNTER: FALLTHROUGH;
  case METRICS_TYPE_GAUGE:
  {
    const char *labels = format_labels(entry->labels);
    if (strlen(labels) > 0) {
      buf_add_printf(data, "%s{%s} %" PRIi64 "\n", entry->name,
                     labels,
                     metrics_store_entry_get_value(entry));
    } else {
      buf_add_printf(data, "%s %" PRIi64 "\n", entry->name,
                     metrics_store_entry_get_value(entry));
    }
    break;
  }
  case METRICS_TYPE_HISTOGRAM:
    format_histogram(entry, data);
    break;
  default:
    tor_assert_unreached();
  }
}
