/* Copyright (c) 2020, The Tor Project, Inc. */
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

/** Return a static buffer pointer that contains a formatted label on the form
 * of key=value.
 *
 * Subsequent call to this function invalidates the previous buffer. */
static const char *
format_label(const char *key, const char *value)
{
  static char buf[128];
  tor_snprintf(buf, sizeof(buf), "%s=%s", key, value);
  return buf;
}

/** Initialize a metrics store for the given service.
 *
 * Essentially, this goes over the base_metrics array and adds them all to the
 * store set with their label(s) if any. */
static void
init_store(hs_service_t *service)
{
  metrics_store_t *store;

  tor_assert(service);

  store = service->metrics.store;

  for (size_t i = 0; i < base_metrics_size; ++i) {
    metrics_store_entry_t *entry =
      metrics_store_add(store, base_metrics[i].type, base_metrics[i].name,
                        base_metrics[i].help);

    /* Add labels to the entry. */
    metrics_store_entry_add_label(entry,
                        format_label("onion", service->onion_address));
    if (base_metrics[i].port_as_label && service->config.ports) {
      SMARTLIST_FOREACH_BEGIN(service->config.ports,
                              const rend_service_port_config_t *, p) {
        metrics_store_entry_add_label(entry,
                      format_label("port", port_to_str(p->virtual_port)));
      } SMARTLIST_FOREACH_END(p);
    }
  }
}

/** Update the metrics key entry in the store in the given service. The port,
 * if non 0, is used to find the correct metrics entry. The value n is the
 * value used to update the entry. */
void
hs_metrics_update_by_service(const hs_metrics_key_t key,
                             hs_service_t *service, const uint16_t port,
                             int64_t n)
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
    if (port == 0 ||
        metrics_store_entry_has_label(entry,
                            format_label("port", port_to_str(port)))) {
      metrics_store_entry_update(entry, n);
      break;
    }
  } SMARTLIST_FOREACH_END(entry);
}

/** Update the metrics key entry in the store of a service identified by the
 * given identity public key. The port, if non 0, is used to find the correct
 * metrics entry. The value n is the value used to update the entry.
 *
 * This is used by callsite that have access to the key but not the service
 * object so an extra lookup is done to find the service. */
void
hs_metrics_update_by_ident(const hs_metrics_key_t key,
                           const ed25519_public_key_t *ident_pk,
                           const uint16_t port, int64_t n)
{
  hs_service_t *service;

  tor_assert(ident_pk);

  service = hs_service_find(ident_pk);
  if (!service) {
    /* This is possible because an onion service client can end up here due to
     * having an identity key onto a connection _to_ an onion service. We
     * can't differentiate that from an actual onion service initiated by a
     * service and thus the only way to know is to lookup the service. */
    return;
  }
  hs_metrics_update_by_service(key, service, port, n);
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
