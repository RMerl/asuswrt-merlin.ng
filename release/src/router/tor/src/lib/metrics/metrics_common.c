/* 2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file metrics_common.c
 * @brief Common code for the metrics library
 **/

#include <stddef.h>

#include "orconfig.h"

#include "lib/log/util_bug.h"
#include "lib/string/printf.h"

#include "lib/metrics/metrics_common.h"

/** Return string representation of a metric type. */
const char *
metrics_type_to_str(const metrics_type_t type)
{
  switch (type) {
  case METRICS_TYPE_COUNTER:
    return "counter";
  case METRICS_TYPE_GAUGE:
    return "gauge";
  case METRICS_TYPE_HISTOGRAM:
    return "histogram";
  default:
    tor_assert_unreached();
  }
}

/** Return a static buffer pointer that contains a formatted label on the form
 * of key=value.
 *
 * Subsequent call to this function invalidates the previous buffer. */
const char *
metrics_format_label(const char *key, const char *value)
{
  static char buf[128];
  tor_snprintf(buf, sizeof(buf), "%s=\"%s\"", key, value);
  return buf;
}
