/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"
#include "app/main/subsysmgr.h"

#include "lib/dispatch/dispatch_naming.h"
#include "lib/dispatch/msgtypes.h"
#include "lib/err/torerr.h"
#include "lib/log/log.h"
#include "lib/malloc/malloc.h"
#include "lib/pubsub/pubsub_build.h"
#include "lib/pubsub/pubsub_connect.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * True iff we have checked tor_subsystems for consistency.
 **/
static bool subsystem_array_validated = false;

/**
 * True if a given subsystem is initialized.  Expand this array if there
 * are more than this number of subsystems.  (We'd rather not
 * dynamically allocate in this module.)
 **/
static bool sys_initialized[128];

/**
 * Exit with a raw assertion if the subsystems list is inconsistent;
 * initialize the subsystem_initialized array.
 **/
static void
check_and_setup(void)
{
  if (subsystem_array_validated)
    return;

  raw_assert(ARRAY_LENGTH(sys_initialized) >= n_tor_subsystems);
  memset(sys_initialized, 0, sizeof(sys_initialized));

  int last_level = MIN_SUBSYS_LEVEL;

  for (unsigned i = 0; i < n_tor_subsystems; ++i) {
    const subsys_fns_t *sys = tor_subsystems[i];
    if (sys->level < MIN_SUBSYS_LEVEL || sys->level > MAX_SUBSYS_LEVEL) {
      fprintf(stderr, "BUG: Subsystem %s (at %u) has an invalid level %d. "
              "It is supposed to be between %d and %d (inclusive).\n",
              sys->name, i, sys->level, MIN_SUBSYS_LEVEL, MAX_SUBSYS_LEVEL);
      raw_assert_unreached_msg("There is a bug in subsystem_list.c");
    }
    if (sys->level < last_level) {
      fprintf(stderr, "BUG: Subsystem %s (at #%u) is in the wrong position. "
              "Its level is %d; but the previous subsystem's level was %d.\n",
              sys->name, i, sys->level, last_level);
      raw_assert_unreached_msg("There is a bug in subsystem_list.c");
    }
    last_level = sys->level;
  }

  subsystem_array_validated = true;
}

/**
 * Initialize all the subsystems; exit on failure.
 **/
int
subsystems_init(void)
{
  return subsystems_init_upto(MAX_SUBSYS_LEVEL);
}

/**
 * Initialize all the subsystems whose level is less than or equal to
 * <b>target_level</b>; exit on failure.
 **/
int
subsystems_init_upto(int target_level)
{
  check_and_setup();

  for (unsigned i = 0; i < n_tor_subsystems; ++i) {
    const subsys_fns_t *sys = tor_subsystems[i];
    if (!sys->supported)
      continue;
    if (sys->level > target_level)
      break;
    if (sys_initialized[i])
      continue;
    int r = 0;
    if (sys->initialize) {
      // Note that the logging subsystem is designed so that it does no harm
      // to log a message in an uninitialized state.  These messages will be
      // discarded for now, however.
      log_debug(LD_GENERAL, "Initializing %s", sys->name);
      r = sys->initialize();
    }
    if (r < 0) {
      fprintf(stderr, "BUG: subsystem %s (at %u) initialization failed.\n",
              sys->name, i);
      raw_assert_unreached_msg("A subsystem couldn't be initialized.");
    }
    sys_initialized[i] = true;
  }

  return 0;
}

/**
 * Add publish/subscribe relationships to <b>builder</b> for all
 * initialized subsystems of level no more than <b>target_level</b>.
 **/
int
subsystems_add_pubsub_upto(pubsub_builder_t *builder,
                           int target_level)
{
  for (unsigned i = 0; i < n_tor_subsystems; ++i) {
    const subsys_fns_t *sys = tor_subsystems[i];
    if (!sys->supported)
      continue;
    if (sys->level > target_level)
      break;
    if (! sys_initialized[i])
      continue;
    int r = 0;
    if (sys->add_pubsub) {
      subsys_id_t sysid = get_subsys_id(sys->name);
      raw_assert(sysid != ERROR_ID);
      pubsub_connector_t *connector;
      connector = pubsub_connector_for_subsystem(builder, sysid);
      r = sys->add_pubsub(connector);
      pubsub_connector_free(connector);
    }
    if (r < 0) {
      fprintf(stderr, "BUG: subsystem %s (at %u) could not connect to "
              "publish/subscribe system.", sys->name, sys->level);
      raw_assert_unreached_msg("A subsystem couldn't be connected.");
    }
  }

  return 0;
}

/**
 * Add publish/subscribe relationships to <b>builder</b> for all
 * initialized subsystems.
 **/
int
subsystems_add_pubsub(pubsub_builder_t *builder)
{
  return subsystems_add_pubsub_upto(builder, MAX_SUBSYS_LEVEL);
}

/**
 * Shut down all the subsystems.
 **/
void
subsystems_shutdown(void)
{
  subsystems_shutdown_downto(MIN_SUBSYS_LEVEL - 1);
}

/**
 * Shut down all the subsystems whose level is above <b>target_level</b>.
 **/
void
subsystems_shutdown_downto(int target_level)
{
  check_and_setup();

  for (int i = (int)n_tor_subsystems - 1; i >= 0; --i) {
    const subsys_fns_t *sys = tor_subsystems[i];
    if (!sys->supported)
      continue;
    if (sys->level <= target_level)
      break;
    if (! sys_initialized[i])
      continue;
    if (sys->shutdown) {
      log_debug(LD_GENERAL, "Shutting down %s", sys->name);
      sys->shutdown();
    }
    sys_initialized[i] = false;
  }
}

/**
 * Run pre-fork code on all subsystems that declare any
 **/
void
subsystems_prefork(void)
{
  check_and_setup();

  for (int i = (int)n_tor_subsystems - 1; i >= 0; --i) {
    const subsys_fns_t *sys = tor_subsystems[i];
    if (!sys->supported)
      continue;
    if (! sys_initialized[i])
      continue;
    if (sys->prefork) {
      log_debug(LD_GENERAL, "Pre-fork: %s", sys->name);
      sys->prefork();
    }
  }
}

/**
 * Run post-fork code on all subsystems that declare any
 **/
void
subsystems_postfork(void)
{
  check_and_setup();

  for (unsigned i = 0; i < n_tor_subsystems; ++i) {
    const subsys_fns_t *sys = tor_subsystems[i];
    if (!sys->supported)
      continue;
    if (! sys_initialized[i])
      continue;
    if (sys->postfork) {
      log_debug(LD_GENERAL, "Post-fork: %s", sys->name);
      sys->postfork();
    }
  }
}

/**
 * Run thread-cleanup code on all subsystems that declare any
 **/
void
subsystems_thread_cleanup(void)
{
  check_and_setup();

  for (int i = (int)n_tor_subsystems - 1; i >= 0; --i) {
    const subsys_fns_t *sys = tor_subsystems[i];
    if (!sys->supported)
      continue;
    if (! sys_initialized[i])
      continue;
    if (sys->thread_cleanup) {
      log_debug(LD_GENERAL, "Thread cleanup: %s", sys->name);
      sys->thread_cleanup();
    }
  }
}
