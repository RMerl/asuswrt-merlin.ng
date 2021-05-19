/* Copyright (c) 2020, The Tor Project, Inc. */
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
  tor_snprintf(buf, sizeof(buf), "{%s}", line);

 end:
  tor_free(line);
  return buf;
}

/** Format the given entry in to the buffer data. */
void
prometheus_format_store_entry(const metrics_store_entry_t *entry, buf_t *data)
{
  tor_assert(entry);
  tor_assert(data);

  buf_add_printf(data, "# HELP %s %s\n", entry->name, entry->help);
  buf_add_printf(data, "# TYPE %s %s\n", entry->name,
                 metrics_type_to_str(entry->type));
  buf_add_printf(data, "%s%s %" PRIi64 "\n", entry->name,
                 format_labels(entry->labels),
                 metrics_store_entry_get_value(entry));
}
