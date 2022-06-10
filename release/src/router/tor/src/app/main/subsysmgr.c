/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file subsysmgr.c
 * @brief Manager for Tor's subsystems.
 *
 * This code is responsible for initializing, configuring, and shutting
 * down all of Tor's individual subsystems.
 **/

#include "orconfig.h"
#include "app/main/subsysmgr.h"

#include "lib/confmgt/confmgt.h"
#include "lib/dispatch/dispatch_naming.h"
#include "lib/dispatch/msgtypes.h"
#include "lib/err/torerr.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"
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

/** Index value indicating that a subsystem has no options/state object, and
 * so that object does not have an index. */
#define IDX_NONE (-1)

/**
 * Runtime status of a single subsystem.
 **/
typedef struct subsys_status_t {
  /** True if the given subsystem is initialized. */
  bool initialized;
  /** Index for this subsystem's options object, or IDX_NONE for none. */
  int options_idx;
  /** Index for this subsystem's state object, or IDX_NONE for none. */
  int state_idx;
} subsys_status_t;

/** An overestimate of the number of subsystems. */
#define N_SYS_STATUS 128
/**
 * True if a given subsystem is initialized.  Expand this array if there
 * are more than this number of subsystems.  (We'd rather not
 * dynamically allocate in this module.)
 **/
static subsys_status_t sys_status[N_SYS_STATUS];

/** Set <b>status</b> to a default (not set-up) state. */
static void
subsys_status_clear(subsys_status_t *status)
{
  if (!status)
    return;
  memset(status, 0, sizeof(*status));
  status->initialized = false;
  status->state_idx = IDX_NONE;
  status->options_idx = IDX_NONE;
}

/**
 * Exit with a raw assertion if the subsystems list is inconsistent;
 * initialize the subsystem_initialized array.
 **/
static void
check_and_setup(void)
{
  if (subsystem_array_validated)
    return;

  raw_assert(ARRAY_LENGTH(sys_status) >= n_tor_subsystems);
  memset(sys_status, 0, sizeof(sys_status));

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
    subsys_status_clear(&sys_status[i]);

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
    if (sys_status[i].initialized)
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
    sys_status[i].initialized = true;
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
    if (! sys_status[i].initialized)
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
    if (! sys_status[i].initialized)
      continue;
    if (sys->shutdown) {
      log_debug(LD_GENERAL, "Shutting down %s", sys->name);
      sys->shutdown();
    }
    subsys_status_clear(&sys_status[i]);
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
    if (! sys_status[i].initialized)
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
    if (! sys_status[i].initialized)
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
    if (! sys_status[i].initialized)
      continue;
    if (sys->thread_cleanup) {
      log_debug(LD_GENERAL, "Thread cleanup: %s", sys->name);
      sys->thread_cleanup();
    }
  }
}

/**
 * Dump a human- and machine-readable list of all the subsystems to stdout,
 * in their initialization order, prefixed with their level.
 **/
void
subsystems_dump_list(void)
{
  for (unsigned i = 0; i < n_tor_subsystems; ++i) {
    const subsys_fns_t *sys = tor_subsystems[i];
    printf("% 4d\t%16s\t%s\n", sys->level, sys->name,
           sys->location?sys->location:"");
  }
}

/**
 * Register all subsystem-declared options formats in <b>mgr</b>.
 *
 * Return 0 on success, -1 on failure.
 **/
int
subsystems_register_options_formats(config_mgr_t *mgr)
{
  tor_assert(mgr);
  check_and_setup();

  for (unsigned i = 0; i < n_tor_subsystems; ++i) {
    const subsys_fns_t *sys = tor_subsystems[i];
    if (sys->options_format) {
      int options_idx = config_mgr_add_format(mgr, sys->options_format);
      sys_status[i].options_idx = options_idx;
      log_debug(LD_CONFIG, "Added options format for %s with index %d",
                sys->name, options_idx);
    }
  }
  return 0;
}

/**
 * Register all subsystem-declared state formats in <b>mgr</b>.
 *
 * Return 0 on success, -1 on failure.
 **/
int
subsystems_register_state_formats(config_mgr_t *mgr)
{
  tor_assert(mgr);
  check_and_setup();

  for (unsigned i = 0; i < n_tor_subsystems; ++i) {
    const subsys_fns_t *sys = tor_subsystems[i];
    if (sys->state_format) {
      int state_idx = config_mgr_add_format(mgr, sys->state_format);
      sys_status[i].state_idx = state_idx;
      log_debug(LD_CONFIG, "Added state format for %s with index %d",
                sys->name, state_idx);
    }
  }
  return 0;
}

#ifdef TOR_UNIT_TESTS
/**
 * Helper: look up the index for <b>sys</b>.  Return -1 if the subsystem
 * is not recognized.
 **/
static int
subsys_get_idx(const subsys_fns_t *sys)
{
  for (unsigned i = 0; i < n_tor_subsystems; ++i) {
    if (sys == tor_subsystems[i])
      return (int)i;
  }
  return -1;
}

/**
 * Return the current state-manager's index for any state held by the
 * subsystem <b>sys</b>.  If <b>sys</b> has no options, return -1.
 *
 * Using raw indices can be error-prone: only do this from the unit
 * tests. If you need a way to access another subsystem's configuration,
 * that subsystem should provide access functions.
 **/
int
subsystems_get_options_idx(const subsys_fns_t *sys)
{
  int i = subsys_get_idx(sys);
  tor_assert(i >= 0);
  return sys_status[i].options_idx;
}

/**
 * Return the current state-manager's index for any state held by the
 * subsystem <b>sys</b>.  If <b>sys</b> has no state, return -1.
 *
 * Using raw indices can be error-prone: only do this from the unit
 * tests.  If you need a way to access another subsystem's state
 * that subsystem should provide access functions.
 **/
int
subsystems_get_state_idx(const subsys_fns_t *sys)
{
  int i = subsys_get_idx(sys);
  tor_assert(i >= 0);
  return sys_status[i].state_idx;
}
#endif /* defined(TOR_UNIT_TESTS) */

/**
 * Call all appropriate set_options() methods to tell the various subsystems
 * about a new set of torrc options.  Return 0 on success, -1 on
 * nonrecoverable failure.
 **/
int
subsystems_set_options(const config_mgr_t *mgr, struct or_options_t *options)
{
  /* XXXX This does not yet handle reversible option assignment; I'll
   * do that later in this branch. */

  for (unsigned i = 0; i < n_tor_subsystems; ++i) {
    const subsys_fns_t *sys = tor_subsystems[i];
    if (sys_status[i].options_idx >= 0 && sys->set_options) {
      void *obj = config_mgr_get_obj_mutable(mgr, options,
                                             sys_status[i].options_idx);
      int rv = sys->set_options(obj);
      if (rv < 0) {
        log_err(LD_CONFIG, "Error when handling option for %s; "
                "cannot proceed.", sys->name);
        return -1;
      }
    }
  }
  return 0;
}

/**
 * Call all appropriate set_state() methods to tell the various subsystems
 * about an initial DataDir/state file.  Return 0 on success, -1 on
 * nonrecoverable failure.
 **/
int
subsystems_set_state(const config_mgr_t *mgr, struct or_state_t *state)
{
  for (unsigned i = 0; i < n_tor_subsystems; ++i) {
    const subsys_fns_t *sys = tor_subsystems[i];
    if (sys_status[i].state_idx >= 0 && sys->set_state) {
      void *obj = config_mgr_get_obj_mutable(mgr, state,
                                             sys_status[i].state_idx);
      int rv = sys->set_state(obj);
      if (rv < 0) {
        log_err(LD_CONFIG, "Error when handling state for %s; "
                "cannot proceed.", sys->name);
        return -1;
      }
    }
  }
  return 0;
}

/**
 * Call all appropriate flush_state() methods to tell the various subsystems
 * to update the state objects in <b>state</b>.  Return 0 on success,
 * -1 on failure.
 **/
int
subsystems_flush_state(const config_mgr_t *mgr, struct or_state_t *state)
{
  int result = 0;
  for (unsigned i = 0; i < n_tor_subsystems; ++i) {
    const subsys_fns_t *sys = tor_subsystems[i];
    if (sys_status[i].state_idx >= 0 && sys->flush_state) {
      void *obj = config_mgr_get_obj_mutable(mgr, state,
                                             sys_status[i].state_idx);
      int rv = sys->flush_state(obj);
      if (rv < 0) {
        log_warn(LD_CONFIG, "Error when flushing state to state object for %s",
                sys->name);
        result = -1;
      }
    }
  }
  return result;
}
