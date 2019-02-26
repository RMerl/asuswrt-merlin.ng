/* Copyright (c) 2017-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file events.h
 * \brief Header file for Tor event tracing.
 **/

#ifndef TOR_TRACE_EVENTS_H
#define TOR_TRACE_EVENTS_H

/*
 * The following defines a generic event tracing function name that has to be
 * used to trace events in the code base.
 *
 * That generic function is then defined by a event tracing framework. For
 * instance, the "log debug" framework sends all trace events to log_debug()
 * which is defined in src/trace/debug.h which can only be enabled at compile
 * time (--enable-event-tracing-debug).
 *
 * By default, every trace events in the code base are replaced by a NOP. See
 * doc/HACKING/Tracing.md for more information on how to use event tracing or
 * add events.
 */

#ifdef TOR_EVENT_TRACING_ENABLED
/* Map every trace event to a per subsystem macro. */
#define tor_trace(subsystem, name, ...) \
  tor_trace_##subsystem(name, __VA_ARGS__)

/* Enable event tracing for the debug framework where all trace events are
 * mapped to a log_debug(). */
#ifdef USE_EVENT_TRACING_DEBUG
#include "lib/trace/debug.h"
#endif

#else /* TOR_EVENT_TRACING_ENABLED */

/* Reaching this point, we NOP every event declaration because event tracing
 * is not been enabled at compile time. */
#define tor_trace(subsystem, name, args...)

#endif /* TOR_EVENT_TRACING_ENABLED */

#endif /* TOR_TRACE_EVENTS_H */
