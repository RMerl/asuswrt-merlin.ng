/* Copyright (c) 2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file trace_probes_cc.c
 * \brief Tracepoint provider source file for the cc subsystem. Probes
 *        are generated within this C file for LTTng-UST
 **/

#include "orconfig.h"

/*
 * Following section is specific to LTTng-UST.
 */
#ifdef USE_TRACING_INSTRUMENTATION_LTTNG

/* Header files that the probes need. */
#include "core/or/or.h"
#include "core/or/channel.h"
#include "core/or/circuit_st.h"
#include "core/or/circuitlist.h"
#include "core/or/congestion_control_common.h"
#include "core/or/congestion_control_st.h"
#include "core/or/connection_st.h"
#include "core/or/edge_connection_st.h"
#include "core/or/or_circuit_st.h"
#include "core/or/origin_circuit_st.h"

#define TRACEPOINT_DEFINE
#define TRACEPOINT_CREATE_PROBES

#include "core/or/trace_probes_cc.h"

#endif /* defined(USE_TRACING_INSTRUMENTATION_LTTNG) */
