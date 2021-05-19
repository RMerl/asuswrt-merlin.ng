/* 2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file metrics_common.c
 * @brief Common code for the metrics library
 **/

#include <stddef.h>

#include "orconfig.h"

#include "lib/log/util_bug.h"

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
  default:
    tor_assert_unreached();
  }
}
