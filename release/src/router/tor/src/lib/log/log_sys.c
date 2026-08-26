/* Copyright (c) 2018-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file log_sys.c
 * \brief Setup and tear down the logging module.
 **/

#include "orconfig.h"
#include "lib/subsys/subsys.h"
#include "lib/log/escape.h"
#include "lib/log/log.h"
#include "lib/log/log_sys.h"
#include "lib/log/util_bug.h"
#include "lib/metrics/metrics_store.h"

static metrics_store_t *the_store;

static int
subsys_logging_initialize(void)
{
  init_logging(0);
  the_store = metrics_store_new();
  return 0;
}

static void
subsys_logging_shutdown(void)
{
  logs_free_all();
  escaped(NULL);
}

static const smartlist_t *
logging_metrics_get_stores(void)
{
  static smartlist_t *stores_list = NULL;

  metrics_store_reset(the_store);

  metrics_store_entry_t *sentry = metrics_store_add(
      the_store,
      METRICS_TYPE_COUNTER,
      METRICS_NAME(bug_reached_count),
      "Total number of BUG() and similar assertion reached",
      0, NULL);
  metrics_store_entry_update(sentry, tor_bug_get_count());

  if (!stores_list) {
    stores_list = smartlist_new();
    smartlist_add(stores_list, the_store);
  }

  return stores_list;
}

const subsys_fns_t sys_logging = {
  .name = "log",
  SUBSYS_DECLARE_LOCATION(),
  .supported = true,
  /* Logging depends on threads, approx time, raw logging, and security.
   * Most other lib modules depend on logging. */
  .level = -90,
  .initialize = subsys_logging_initialize,
  .shutdown = subsys_logging_shutdown,
  .get_metrics = logging_metrics_get_stores,
};
