#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
#ifndef __buzzz_kevt_h_included__
#define __buzzz_kevt_h_included__

#if defined(CONFIG_BUZZZ_KEVT) || defined(CONFIG_BUZZZ_FUNC)
/*
 * +----------------------------------------------------------------------------
 *
 * BCM BUZZZ ARM Cortex A9 Router Kernel events
 *
 * $Copyright Open Broadcom Corporation$
 * $Id$
 *
 * vim: set ts=4 noet sw=4 tw=80:
 * -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * +----------------------------------------------------------------------------
 */

#include <uapi/linux/buzzz.h>

#undef BUZZZ_KEVT
#define BUZZZ_KEVT(event)       BUZZZ_KEVT__ ## event,

#undef  _B_
#undef  _H_
#undef  _N_
#undef  _FAIL_
#define _B_                     "\e[0;34m"
#define _H_                     "\e[0;31m;40m"
#define _N_                     "\e[0m"
#define _FAIL_                  _H_ " === FAILURE ===" _N_

/* Expected events : Font = Normal */
#define BUZZZ_KEVTN(event, format) \
    buzzz_event_reg(BUZZZ_KEVT__## event, "\t\t" format);

/* Unexpected events: Font = bold2 highlighted */
#define BUZZZ_KEVTH(event, format) \
    buzzz_event_reg(BUZZZ_KEVT__## event, _H_ "\t\t" format _N_);

/**
 * DO NOT SUBMIT USER EVENTS
 *
 * For private debug (not for submission), a user event may be added by:
 * 1. Add an enum entry to buzzz_rtr_dpid,
 *        e.g. BUZZZ_KEVT(HELLO_WORLD)
 *
 * 2. Add an entry to buzzz_dp_init(),
 *        e.g. BUZZZ_KEVTN(HELLO_WORLD,   "hello world at %pS from %pS")
 *
 * 3. In source code base, insert instrumentation:
 *        e.g. BUZZZ_DPL3(HELLO_WORLD, 2,
 *                       BUZZZ32(BUZZZ_CUR_IP_), BUZZZ32(BUZZZ_RET_IP_));
 *
 *    See uapi/linux/buzzz.h, where BUZZZ_DPL tracing level is set to 3, thereby
 *    enabling all instrumentations BUZZZ_DPL1(), BUZZZ_DPL2() and BUZZZ_DPL3() 
 *    Instrumentation with BUZZZ_DPL4() and BUZZZ_DPL5() are compiled out.
 *
 *    Second parameter to BUZZZ_DPL#() specifies the number of arguments to be
 *    logged, in the above example, it is 2 arguments (maximum 3 arguments).
 *    - First argument in example is current instruction address, and
 *    - Second argument is return address.
 *    Arguments are 32bit values. [Gets messy on 64b aarch]
 *
 * Do not forget to invoke, buzzz_dp_init() once ... say in a module init.
 */
typedef
enum buzzz_rtr_dpid
{
    BUZZZ_KEVT__DATAPATH_START = 100,

    BUZZZ_KEVT(SAMPLE)
    /* Define user events here */

} buzzz_rtr_dpid_t;


/* Invoke this once in a datapath module's init */
static inline int
buzzz_dp_init(void)
{
    BUZZZ_KEVTN(SAMPLE,                "sample pkt<%p>")
    /* Add user event logs here */

    return 0;
}
#else  /* ! CONFIG_BUZZZ */
#define BUZZZ_DPL1(ID, N, ARG...)   do {} while (0)
#define BUZZZ_DPL2(ID, N, ARG...)   do {} while (0)
#define BUZZZ_DPL3(ID, N, ARG...)   do {} while (0)
#define BUZZZ_DPL4(ID, N, ARG...)   do {} while (0)
#define BUZZZ_DPL5(ID, N, ARG...)   do {} while (0)
#endif /* ! CONFIG_BUZZZ */

#endif /* __buzzz_kevt_h_included__ */
#endif
