/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#ifndef TOR_SUBSYS_T
#define TOR_SUBSYS_T

#include <stdbool.h>

struct pubsub_connector_t;

/**
 * A subsystem is a part of Tor that is initialized, shut down, configured,
 * and connected to other parts of Tor.
 *
 * All callbacks are optional -- if a callback is set to NULL, the subsystem
 * manager will treat it as a no-op.
 *
 * You should use c99 named-field initializers with this structure: we
 * will be adding more fields, often in the middle of the structure.
 **/
typedef struct subsys_fns_t {
  /**
   * The name of this subsystem.  It should be a programmer-readable
   * identifier.
   **/
  const char *name;

  /**
   * Whether this subsystem is supported -- that is, whether it is compiled
   * into Tor.  For most subsystems, this should be true.
   **/
  bool supported;

  /**
   * The 'initialization level' for the subsystem.  It should run from -100
   * through +100.  The subsystems are initialized from lowest level to
   * highest, and shut down from highest level to lowest.
   **/
  int level;

  /**
   * Initialize any global components of this subsystem.
   *
   * This function MAY rely on any lower-level subsystem being initialized.
   *
   * This function MUST NOT rely on any runtime configuration information;
   * it is only for global state or pre-configuration state.
   *
   * (If you need to do any setup that depends on configuration, you'll need
   * to declare a configuration callback. (Not yet designed))
   *
   * This function MUST NOT have any parts that can fail.
   **/
  int (*initialize)(void);

  /**
   * Connect a subsystem to the message dispatch system.
   **/
  int (*add_pubsub)(struct pubsub_connector_t *);

  /**
   * Perform any necessary pre-fork cleanup.  This function may not fail.
   */
  void (*prefork)(void);

  /**
   * Perform any necessary post-fork setup. This function may not fail.
   */
  void (*postfork)(void);

  /**
   * Free any thread-local resources held by this subsystem. Called before
   * the thread exits.
   */
  void (*thread_cleanup)(void);

  /**
   * Free all resources held by this subsystem.
   *
   * This function is not allowed to fail.
   **/
  void (*shutdown)(void);

} subsys_fns_t;

#define MIN_SUBSYS_LEVEL -100
#define MAX_SUBSYS_LEVEL 100

/* All tor "libraries" (in src/libs) should have a subsystem level equal to or
 * less than this value. */
#define SUBSYS_LEVEL_LIBS -10

#endif /* !defined(TOR_SUBSYS_T) */
