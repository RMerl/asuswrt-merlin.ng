/* Copyright (c) 2017-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file debug.h
 * \brief Macros for debugging our event-trace support.
 **/

#ifndef TOR_TRACE_LOG_DEBUG_H
#define TOR_TRACE_LOG_DEBUG_H

#include "lib/log/log.h"

/* Stringify pre-processor trick. */
#define XSTR(d) STR(d)
#define STR(s) #s

/* Send every event to a debug log level. This is useful to debug new trace
 * events without implementing them for a specific event tracing framework.
 * Note that the arguments are ignored since at this step we do not know the
 * types and amount there is. */

/* Example on how to map a tracepoint to log_debug(). */
#undef tor_trace
#define tor_trace(subsystem, name, args...) \
  log_debug(LD_GENERAL, "Trace event \"" XSTR(name) "\" from " \
                        "\"" XSTR(subsystem) "\" hit. " \
                        "(line "XSTR(__LINE__) ")")

#endif /* !defined(TOR_TRACE_LOG_DEBUG_H) */
