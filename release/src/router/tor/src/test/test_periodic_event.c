/* Copyright (c) 2018-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_periodic_event.c
 * \brief Test the periodic events that Tor uses for different roles. They are
 *        part of the libevent mainloop
 */

#define CONFIG_PRIVATE
#define HS_SERVICE_PRIVATE
#define MAINLOOP_PRIVATE

#include "test/test.h"
#include "test/test_helpers.h"

#include "core/or/or.h"
#include "app/config/config.h"
#include "feature/hibernate/hibernate.h"
#include "feature/hs/hs_metrics.h"
#include "feature/hs/hs_service.h"
#include "core/mainloop/mainloop.h"
#include "core/mainloop/netstatus.h"
#include "core/mainloop/periodic.h"

/** Helper function: This is replaced in some tests for the event callbacks so
 * we don't actually go into the code path of those callbacks. */
static int
dumb_event_fn(time_t now, const or_options_t *options)
{
  (void) now;
  (void) options;

  /* Will get rescheduled in 300 seconds. It just can't be 0. */
  return 300;
}

static void
register_dummy_hidden_service(hs_service_t *service)
{
  memset(service, 0, sizeof(hs_service_t));
  memset(&service->keys.identity_pk, 'A', sizeof(service->keys.identity_pk));
  (void) register_service(get_hs_service_map(), service);
}

static void
test_pe_initialize(void *arg)
{
  (void) arg;

  /* Initialize the events but the callback won't get called since we would
   * need to run the main loop and then wait for a second delaying the unit
   * tests. Instead, we'll test the callback work independently elsewhere. */
  initialize_periodic_events();
  periodic_events_connect_all();
  set_network_participation(false);
  rescan_periodic_events(get_options());

  /* Validate that all events have been set up. */
  for (int i = 0; mainloop_periodic_events[i].name; ++i) {
    periodic_event_item_t *item = &mainloop_periodic_events[i];
    tt_assert(item->ev);
    tt_assert(item->fn);
    tt_u64_op(item->last_action_time, OP_EQ, 0);
    /* Every event must have role(s) assign to it. This is done statically. */
    tt_u64_op(item->roles, OP_NE, 0);
    int should_be_enabled = (item->roles & PERIODIC_EVENT_ROLE_ALL) &&
      !(item->flags & PERIODIC_EVENT_FLAG_NEED_NET);
    tt_uint_op(periodic_event_is_enabled(item), OP_EQ, should_be_enabled);
  }

 done:
  teardown_periodic_events();
}

static void
test_pe_launch(void *arg)
{
  hs_service_t service, *to_remove = NULL;
  or_options_t *options;

  (void) arg;

  hs_init();
  /* We need to put tor in hibernation live state so the events requiring
   * network gets enabled. */
  consider_hibernation(time(NULL));

  set_network_participation(true);

  /* Hack: We'll set a dumb fn() of each events so they don't get called when
   * dispatching them. We just want to test the state of the callbacks, not
   * the whole code path. */
  for (int i = 0; mainloop_periodic_events[i].name; ++i) {
    periodic_event_item_t *item = &mainloop_periodic_events[i];
    item->fn = dumb_event_fn;
  }

  options = get_options_mutable();
  options->SocksPort_set = 1;
  periodic_events_on_new_options(options);

#if 0
  /* Lets make sure that before initialization, we can't scan the periodic
   * events list and launch them. Lets try by being a Client. */
  /* XXXX We make sure these events are initialized now way earlier than we
   * did before. */
  for (int i = 0; periodic_events[i].name; ++i) {
    periodic_event_item_t *item = &periodic_events[i];
    tt_int_op(periodic_event_is_enabled(item), OP_EQ, 0);
  }
#endif /* 0 */

  initialize_periodic_events();
  periodic_events_connect_all();

  /* Now that we've initialized, rescan the list to launch. */
  periodic_events_on_new_options(options);

  int mask = PERIODIC_EVENT_ROLE_CLIENT|PERIODIC_EVENT_ROLE_ALL|
    PERIODIC_EVENT_ROLE_NET_PARTICIPANT;
  for (int i = 0; mainloop_periodic_events[i].name; ++i) {
    periodic_event_item_t *item = &mainloop_periodic_events[i];
    int should_be_enabled = !!(item->roles & mask);
    tt_int_op(periodic_event_is_enabled(item), OP_EQ, should_be_enabled);
    // enabled or not, the event has not yet been run.
    tt_u64_op(item->last_action_time, OP_EQ, 0);
  }

  /* Remove Client but become a Relay. */
  options->SocksPort_set = 0;
  options->ORPort_set = 1;
  periodic_events_on_new_options(options);

  unsigned roles = get_my_roles(options);
  tt_uint_op(roles, OP_EQ,
             PERIODIC_EVENT_ROLE_RELAY|PERIODIC_EVENT_ROLE_DIRSERVER|
             PERIODIC_EVENT_ROLE_ALL|PERIODIC_EVENT_ROLE_NET_PARTICIPANT);

  for (int i = 0; mainloop_periodic_events[i].name; ++i) {
    periodic_event_item_t *item = &mainloop_periodic_events[i];
    /* Only Client role should be disabled. */
    if (item->roles == PERIODIC_EVENT_ROLE_CLIENT) {
      tt_int_op(periodic_event_is_enabled(item), OP_EQ, 0);
    }
    if (item->roles & PERIODIC_EVENT_ROLE_RELAY) {
      tt_int_op(periodic_event_is_enabled(item), OP_EQ, 1);
    }
    /* Non Relay role should be disabled, except for Dirserver. */
    if (!(item->roles & roles)) {
      tt_int_op(periodic_event_is_enabled(item), OP_EQ, 0);
    }
  }

  /* Disable everything and we'll enable them ALL. */
  options->SocksPort_set = 0;
  options->ORPort_set = 0;
  options->DisableNetwork = 1;
  set_network_participation(false);
  periodic_events_on_new_options(options);

  for (int i = 0; mainloop_periodic_events[i].name; ++i) {
    periodic_event_item_t *item = &mainloop_periodic_events[i];
    int should_be_enabled = (item->roles & PERIODIC_EVENT_ROLE_ALL) &&
      !(item->flags & PERIODIC_EVENT_FLAG_NEED_NET);
    tt_int_op(periodic_event_is_enabled(item), OP_EQ, should_be_enabled);
  }

  /* Enable everything. */
  options->SocksPort_set = 1; options->ORPort_set = 1;
  options->BridgeRelay = 1; options->AuthoritativeDir = 1;
  options->V3AuthoritativeDir = 1; options->BridgeAuthoritativeDir = 1;
  options->DisableNetwork = 0;
  set_network_participation(true);
  register_dummy_hidden_service(&service);
  periodic_events_on_new_options(options);
  /* Note down the reference because we need to remove this service from the
   * global list before the hs_free_all() call so it doesn't try to free
   * memory on the stack. Furthermore, we can't remove it now else it will
   * trigger a rescan of the event disabling the HS service event. */
  to_remove = &service;

  for (int i = 0; mainloop_periodic_events[i].name; ++i) {
    periodic_event_item_t *item = &mainloop_periodic_events[i];
    tt_int_op(periodic_event_is_enabled(item), OP_EQ,
              (item->roles != PERIODIC_EVENT_ROLE_CONTROLEV));
  }

 done:
  if (to_remove) {
    hs_metrics_service_free(&service);
    remove_service(get_hs_service_map(), to_remove);
  }
  hs_free_all();
}

static void
test_pe_get_roles(void *arg)
{
  int roles;

  (void) arg;

  /* Just so the HS global map exists. */
  hs_init();

  or_options_t *options = get_options_mutable();
  tt_assert(options);
  set_network_participation(true);

  const int ALL = PERIODIC_EVENT_ROLE_ALL |
    PERIODIC_EVENT_ROLE_NET_PARTICIPANT;

  /* Nothing configured, should be no roles. */
  tt_assert(net_is_disabled());
  roles = get_my_roles(options);
  tt_int_op(roles, OP_EQ, ALL);

  /* Indicate we have a SocksPort, roles should be come Client. */
  options->SocksPort_set = 1;
  roles = get_my_roles(options);
  tt_int_op(roles, OP_EQ, PERIODIC_EVENT_ROLE_CLIENT|ALL);

  /* Now, we'll add a ORPort so should now be a Relay + Client. */
  options->ORPort_set = 1;
  roles = get_my_roles(options);
  tt_int_op(roles, OP_EQ,
            (PERIODIC_EVENT_ROLE_CLIENT | PERIODIC_EVENT_ROLE_RELAY |
             PERIODIC_EVENT_ROLE_DIRSERVER | ALL));

  /* Now add a Bridge. */
  options->BridgeRelay = 1;
  roles = get_my_roles(options);
  tt_int_op(roles, OP_EQ,
            (PERIODIC_EVENT_ROLE_CLIENT | PERIODIC_EVENT_ROLE_RELAY |
             PERIODIC_EVENT_ROLE_BRIDGE | PERIODIC_EVENT_ROLE_DIRSERVER |
             ALL));
  tt_assert(roles & PERIODIC_EVENT_ROLE_ROUTER);
  /* Unset client so we can solely test Router role. */
  options->SocksPort_set = 0;
  roles = get_my_roles(options);
  tt_int_op(roles, OP_EQ,
            PERIODIC_EVENT_ROLE_ROUTER | PERIODIC_EVENT_ROLE_DIRSERVER |
            ALL);

  /* Reset options so we can test authorities. */
  options->SocksPort_set = 0;
  options->ORPort_set = 0;
  options->BridgeRelay = 0;
  roles = get_my_roles(options);
  tt_int_op(roles, OP_EQ, ALL);

  /* Now upgrade to Dirauth. */
  options->DirPort_set = 1;
  options->AuthoritativeDir = 1;
  options->V3AuthoritativeDir = 1;
  roles = get_my_roles(options);
  tt_int_op(roles, OP_EQ,
            PERIODIC_EVENT_ROLE_DIRAUTH|PERIODIC_EVENT_ROLE_DIRSERVER|ALL);
  tt_assert(roles & PERIODIC_EVENT_ROLE_AUTHORITIES);

  /* Now Bridge Authority. */
  options->V3AuthoritativeDir = 0;
  options->BridgeAuthoritativeDir = 1;
  roles = get_my_roles(options);
  tt_int_op(roles, OP_EQ,
            PERIODIC_EVENT_ROLE_BRIDGEAUTH|PERIODIC_EVENT_ROLE_DIRSERVER|ALL);
  tt_assert(roles & PERIODIC_EVENT_ROLE_AUTHORITIES);

  /* Move that bridge auth to become a relay. */
  options->ORPort_set = 1;
  roles = get_my_roles(options);
  tt_int_op(roles, OP_EQ,
            (PERIODIC_EVENT_ROLE_BRIDGEAUTH | PERIODIC_EVENT_ROLE_RELAY
             | PERIODIC_EVENT_ROLE_DIRSERVER|ALL));
  tt_assert(roles & PERIODIC_EVENT_ROLE_AUTHORITIES);

  /* And now an Hidden service. */
  hs_service_t service;
  register_dummy_hidden_service(&service);
  roles = get_my_roles(options);
  /* Remove it now so the hs_free_all() doesn't try to free stack memory. */
  remove_service(get_hs_service_map(), &service);
  hs_metrics_service_free(&service);
  tt_int_op(roles, OP_EQ,
            (PERIODIC_EVENT_ROLE_BRIDGEAUTH | PERIODIC_EVENT_ROLE_RELAY |
             PERIODIC_EVENT_ROLE_HS_SERVICE | PERIODIC_EVENT_ROLE_DIRSERVER |
             ALL));
  tt_assert(roles & PERIODIC_EVENT_ROLE_AUTHORITIES);

 done:
  hs_free_all();
}

static void
test_pe_hs_service(void *arg)
{
  hs_service_t service, *to_remove = NULL;

  (void) arg;

  hs_init();
  /* We need to put tor in hibernation live state so the events requiring
   * network gets enabled. */
  consider_hibernation(time(NULL));
  /* Initialize the events so we can enable them */
  initialize_periodic_events();
  periodic_events_connect_all();

  /* Hack: We'll set a dumb fn() of each events so they don't get called when
   * dispatching them. We just want to test the state of the callbacks, not
   * the whole code path. */
  for (int i = 0; mainloop_periodic_events[i].name; ++i) {
    periodic_event_item_t *item = &mainloop_periodic_events[i];
    item->fn = dumb_event_fn;
  }

  /* This should trigger a rescan of the list and enable the HS service
   * events. */
  register_dummy_hidden_service(&service);
  /* Note down the reference because we need to remove this service from the
   * global list before the hs_free_all() call so it doesn't try to free
   * memory on the stack. Furthermore, we can't remove it now else it will
   * trigger a rescan of the event disabling the HS service event. */
  to_remove = &service;

  for (int i = 0; mainloop_periodic_events[i].name; ++i) {
    periodic_event_item_t *item = &mainloop_periodic_events[i];
    if (item->roles & PERIODIC_EVENT_ROLE_HS_SERVICE) {
      tt_int_op(periodic_event_is_enabled(item), OP_EQ, 1);
    }
  }
  to_remove = NULL;

  /* Remove the service from the global map, it should trigger a rescan and
   * disable the HS service events. */
  remove_service(get_hs_service_map(), &service);
  hs_metrics_service_free(&service);
  for (int i = 0; mainloop_periodic_events[i].name; ++i) {
    periodic_event_item_t *item = &mainloop_periodic_events[i];
    if (item->roles & PERIODIC_EVENT_ROLE_HS_SERVICE) {
      tt_int_op(periodic_event_is_enabled(item), OP_EQ, 0);
    }
  }

 done:
  if (to_remove) {
    hs_metrics_service_free(&service);
    remove_service(get_hs_service_map(), to_remove);
  }
  hs_free_all();
}

#define PE_TEST(name) \
  { #name, test_pe_## name , TT_FORK, NULL, NULL }

struct testcase_t periodic_event_tests[] = {
  PE_TEST(initialize),
  PE_TEST(launch),
  PE_TEST(get_roles),
  PE_TEST(hs_service),

  END_OF_TESTCASES
};
