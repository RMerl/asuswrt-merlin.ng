/* Copyright (c) 2020-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file hs_metrics.c
 * @brief Onion service metrics exposed through the MetricsPort
 **/

#define HS_METRICS_ENTRY_PRIVATE

#include "orconfig.h"

#include "lib/malloc/malloc.h"
#include "lib/container/smartlist.h"
#include "lib/metrics/metrics_store.h"
#include "lib/log/util_bug.h"

#include "feature/hs/hs_metrics.h"
#include "feature/hs/hs_metrics_entry.h"
#include "feature/hs/hs_service.h"

/** Return a static buffer pointer that contains the port as a string.
 *
 * Subsequent call to this function invalidates the previous buffer. */
static const char *
port_to_str(const uint16_t port)
{
  static char buf[8];
  tor_snprintf(buf, sizeof(buf), "%u", port);
  return buf;
}

/** Add a new metric to the metrics store of the service.
 *
 * <b>metric</b> is the index of the metric in the <b>base_metrics</b> array.
 */
static void
add_metric_with_labels(hs_service_t *service, hs_metrics_key_t metric,
                       bool port_as_label, uint16_t port)
{
  metrics_store_t *store;
  const char **error_reasons = NULL;
  size_t num_error_reasons = 0;

  tor_assert(service);

  if (BUG(metric >= base_metrics_size))
    return;

  store = service->metrics.store;

  /* Check whether the current metric is an error metric, because error metrics
   * require an additional `reason` label. */
  switch (metric) {
    case HS_METRICS_NUM_REJECTED_INTRO_REQ:
      error_reasons = hs_metrics_intro_req_error_reasons;
      num_error_reasons = hs_metrics_intro_req_error_reasons_size;
      break;
    case HS_METRICS_NUM_FAILED_RDV:
      error_reasons = hs_metrics_rend_error_reasons;
      num_error_reasons = hs_metrics_rend_error_reasons_size;
      break;
    /* Fall through for all other metrics, as they don't need a
     * reason label. */
    case HS_METRICS_NUM_INTRODUCTIONS: FALLTHROUGH;
    case HS_METRICS_APP_WRITE_BYTES: FALLTHROUGH;
    case HS_METRICS_APP_READ_BYTES: FALLTHROUGH;
    case HS_METRICS_NUM_ESTABLISHED_RDV: FALLTHROUGH;
    case HS_METRICS_NUM_RDV: FALLTHROUGH;
    case HS_METRICS_NUM_ESTABLISHED_INTRO: FALLTHROUGH;
    case HS_METRICS_POW_NUM_PQUEUE_RDV: FALLTHROUGH;
    case HS_METRICS_POW_SUGGESTED_EFFORT: FALLTHROUGH;
    case HS_METRICS_INTRO_CIRC_BUILD_TIME: FALLTHROUGH;
    case HS_METRICS_REND_CIRC_BUILD_TIME: FALLTHROUGH;
    default:
      break;
  }

  /* We don't need a reason label for this metric */
  if (!num_error_reasons) {
      metrics_store_entry_t *entry = metrics_store_add(
          store, base_metrics[metric].type, base_metrics[metric].name,
          base_metrics[metric].help, base_metrics[metric].bucket_count,
          base_metrics[metric].buckets);

      metrics_store_entry_add_label(entry,
              metrics_format_label("onion", service->onion_address));

      if (port_as_label) {
        metrics_store_entry_add_label(entry,
                metrics_format_label("port", port_to_str(port)));
      }

      return;
  }

  tor_assert(error_reasons);

  /* Add entries with reason as label. We need one metric line per
   * reason. */
  for (size_t i = 0; i < num_error_reasons; ++i) {
    metrics_store_entry_t *entry =
      metrics_store_add(store, base_metrics[metric].type,
                        base_metrics[metric].name,
                        base_metrics[metric].help,
                        base_metrics[metric].bucket_count,
                        base_metrics[metric].buckets);
    /* Add labels to the entry. */
    metrics_store_entry_add_label(entry,
            metrics_format_label("onion", service->onion_address));
    metrics_store_entry_add_label(entry,
            metrics_format_label("reason", error_reasons[i]));

    if (port_as_label) {
      metrics_store_entry_add_label(entry,
              metrics_format_label("port", port_to_str(port)));
    }
  }
}

/** Initialize a metrics store for the given service.
 *
 * Essentially, this goes over the base_metrics array and adds them all to the
 * store set with their label(s) if any. */
static void
init_store(hs_service_t *service)
{
  tor_assert(service);

  for (size_t i = 0; i < base_metrics_size; ++i) {
    /* Add entries with port as label. We need one metric line per port. */
    if (base_metrics[i].port_as_label && service->config.ports) {
      SMARTLIST_FOREACH_BEGIN(service->config.ports,
                              const hs_port_config_t *, p) {
        add_metric_with_labels(service, base_metrics[i].key, true,
                               p->virtual_port);
      } SMARTLIST_FOREACH_END(p);
    } else {
      add_metric_with_labels(service, base_metrics[i].key, false, 0);
    }
  }
}

/** Update the metrics key entry in the store in the given service. The port,
 * if non 0, and the reason label, if non NULL, are used to find the correct
 * metrics entry. The value n is the value used to update the entry. */
void
hs_metrics_update_by_service(const hs_metrics_key_t key,
                             const hs_service_t *service,
                             uint16_t port, const char *reason,
                             int64_t n, int64_t obs, bool reset)
{
  tor_assert(service);

  /* Get the metrics entry in the store. */
  smartlist_t *entries = metrics_store_get_all(service->metrics.store,
                                               base_metrics[key].name);
  if (BUG(!entries)) {
    return;
  }

  /* We need to find the right metrics entry by finding the port label if any.
   *
   * XXX: This is not the most optimal due to the string format. Maybe at some
   * point turn this into a kvline and a map in a metric entry? */
  SMARTLIST_FOREACH_BEGIN(entries, metrics_store_entry_t *, entry) {
    if ((port == 0 ||
         metrics_store_entry_has_label(
             entry, metrics_format_label("port", port_to_str(port)))) &&
        ((!reason || metrics_store_entry_has_label(
                         entry, metrics_format_label("reason", reason))))) {
      if (reset) {
        metrics_store_entry_reset(entry);
      }

      if (metrics_store_entry_is_histogram(entry)) {
        metrics_store_hist_entry_update(entry, n, obs);
      } else {
        metrics_store_entry_update(entry, n);
      }

      break;
    }
  } SMARTLIST_FOREACH_END(entry);
}

/** Update the metrics key entry in the store of a service identified by the
 * given identity public key. The port, if non 0, and the reason label, if non
 * NULL, are used to find the correct metrics entry. The value n is the value
 * used to update the entry.
 *
 * This is used by callsite that have access to the key but not the service
 * object so an extra lookup is done to find the service. */
void
hs_metrics_update_by_ident(const hs_metrics_key_t key,
                           const ed25519_public_key_t *ident_pk,
                           const uint16_t port, const char *reason,
                           int64_t n, int64_t obs, bool reset)
{
  hs_service_t *service;

  if (!ident_pk) {
    /* We can end up here in case this is used from a failure/closing path for
     * which we might not have any identity key attacehed to a circuit or
     * connection yet. Simply don't assume we have one. */
    return;
  }

  service = hs_service_find(ident_pk);
  if (!service) {
    /* This is possible because an onion service client can end up here due to
     * having an identity key onto a connection _to_ an onion service. We
     * can't differentiate that from an actual onion service initiated by a
     * service and thus the only way to know is to lookup the service. */
    return;
  }
  hs_metrics_update_by_service(key, service, port, reason, n, obs, reset);
}

/** Return a list of all the onion service metrics stores. This is the
 * function attached to the .get_metrics() member of the subsys_t. */
const smartlist_t *
hs_metrics_get_stores(void)
{
  /* We can't have the caller to free the returned list so keep it static,
   * simply update it. */
  static smartlist_t *stores_list = NULL;

  smartlist_free(stores_list);
  stores_list = hs_service_get_metrics_stores();
  return stores_list;
}

/** Initialize the metrics store in the given service. */
void
hs_metrics_service_init(hs_service_t *service)
{
  tor_assert(service);

  /* This function is called when we register a service and so it could either
   * be a new service or a service that was just reloaded through a HUP signal
   * for instance. Thus, it is possible that the service has already an
   * initialized store. If so, just return. */
  if (service->metrics.store) {
    return;
  }

  service->metrics.store = metrics_store_new();
  init_store(service);
}

/** Free the metrics store in the given service. */
void
hs_metrics_service_free(hs_service_t *service)
{
  tor_assert(service);

  metrics_store_free(service->metrics.store);
}
