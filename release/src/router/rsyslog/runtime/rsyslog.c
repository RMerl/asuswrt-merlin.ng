/* rsyslog.c - the main entry point into rsyslog's runtime library (RTL)
 *
 * This module contains all function which work on a RTL global level. It's
 * name is abbreviated to "rsrt" (rsyslog runtime).
 *
 * Please note that the runtime library tends to be plugin-safe. That is, it must be
 * initialized by calling a global initialization function. However, that
 * function checks if the library is already initialized and, if so, does
 * nothing except incrementing a refeence count. Similarly, the deinit
 * function does nothing as long as there are still other users (which
 * is tracked via the refcount). As such, it is safe to call init and
 * exit multiple times, as long as this are always matching calls. This
 * capability is needed for a plugin system, where one plugin never
 * knows what the other did. HOWEVER, as of this writing, not all runtime
 * library objects may work cleanly without static global data (the
 * debug system is a very good example of this). So while we aim at the
 * ability to work well in a plugin environment, things may not really work
 * out. If you intend to use the rsyslog runtime library inside plugins,
 * you should investigate the situation in detail. Please note that the
 * rsyslog project itself does not yet need this functionality - thus you
 * can safely assume it is totally untested ;).
 *
 * rgerhards, 2008-04-17: I have now once again checked on the plugin-safety.
 * Unfortunately, there is currently no hook at all with which we could
 * abstract a global data instance class. As such, we can NOT make the
 * runtime plugin-safe in the above-described sense. As the rsyslog
 * project itself does not need this functionality (and it is quesationable
 * if someone else ever will), we do currently do not make an effort to
 * support it. So if you intend to use rsyslog runtime inside a non-rsyslog
 * plugin system, be careful!
 *
 * The rsyslog runtime library is in general reentrant and thread-safe. There
 * are some intentional exceptions (e.g. inside the msg object). These are
 * documented. Any other threading and reentrency issue can be considered a bug.
 *
 * Module begun 2008-04-16 by Rainer Gerhards
 *
 * Copyright 2008-2016 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of the rsyslog runtime library.
 *
 * The rsyslog runtime library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The rsyslog runtime library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the rsyslog runtime library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
 * A copy of the LGPL can be found in the file "COPYING.LESSER" in this distribution.
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#ifdef ENABLE_LIBLOGGING_STDLOG
#include <liblogging/stdlog.h>
#endif

#include "rsyslog.h"
#include "obj.h"
#include "stringbuf.h"
#include "wti.h"
#include "wtp.h"
#include "datetime.h"
#include "queue.h"
#include "conf.h"
#include "rsconf.h"
#include "glbl.h"
#include "errmsg.h"
#include "prop.h"
#include "ruleset.h"
#include "parser.h"
#include "lookup.h"
#include "strgen.h"
#include "statsobj.h"
#include "atomic.h"
#include "srUtils.h"

pthread_attr_t default_thread_attr;
#ifdef HAVE_PTHREAD_SETSCHEDPARAM
struct sched_param default_sched_param;
int default_thr_sched_policy;
#endif

/* globally visible static data - see comment in rsyslog.h for details */
uchar *glblModPath; /* module load path */
void (*glblErrLogger)(const int, const int, const uchar*) = dfltErrLogger;
/* the error logger to use by the errmsg module */

/* static data */
static int iRefCount = 0; /* our refcount - it MUST exist only once inside a process (not thread)
			thus it is perfectly OK to use a static. MUST be initialized to 0! */

/* This is the default instance of the error logger. It simply writes the message
 * to stderr. It is expected that this is replaced by the runtime user very early
 * during startup (at least if the default is unsuitable). However, we provide a
 * default so that we can log errors during the intial phase, most importantly
 * during initialization. -- rgerhards. 2008-04-17
 */
void
dfltErrLogger(const int severity, const int iErr, const uchar *errMsg)
{
	fprintf(stderr, "rsyslog internal message (%d,%d): %s\n", severity, iErr, errMsg);
}


/* set the error log function
 * rgerhards, 2008-04-18
 */
void
rsrtSetErrLogger(void (*errLogger)(const int, const int, const uchar*))
{
	assert(errLogger != NULL);
	glblErrLogger = errLogger;
}


/* globally initialze the runtime system
 * NOTE: this is NOT thread safe and must not be called concurrently. If that
 * ever poses a problem, we may use proper mutex calls - not considered needed yet.
 * If ppErrObj is provided, it receives a char pointer to the name of the object that
 * caused the problem (if one occured). The caller must never free this pointer. If
 * ppErrObj is NULL, no such information will be provided. pObjIF is the pointer to
 * the "obj" object interface, which may be used to query any other rsyslog objects.
 * rgerhards, 2008-04-16
 */
rsRetVal
rsrtInit(const char **ppErrObj, obj_if_t *pObjIF)
{
	DEFiRet;
	int ret;
	char errstr[1024];

	if(iRefCount == 0) {
		seedRandomNumber();
		/* init runtime only if not yet done */
#ifdef ENABLE_LIBLOGGING_STDLOG
		stdlog_init(0);
		stdlog_hdl = stdlog_open("rsyslogd", 0, STDLOG_SYSLOG, NULL);
#endif
		ret = pthread_attr_init(&default_thread_attr);
		if(ret != 0) {
			rs_strerror_r(ret, errstr, sizeof(errstr));
			fprintf(stderr, "rsyslogd: pthread_attr_init failed during "
				"startup - can not continue. Error was %s\n", errstr);
			exit(1);
		}
		pthread_attr_setstacksize(&default_thread_attr, 4096*1024);
#ifdef HAVE_PTHREAD_SETSCHEDPARAM
		ret = pthread_getschedparam(pthread_self(), &default_thr_sched_policy,
			&default_sched_param);
		if(ret != 0) {
			rs_strerror_r(ret, errstr, sizeof(errstr));
			fprintf(stderr, "rsyslogd: pthread_getschedparam failed during "
				"startup - ignoring. Error was %s\n", errstr);
			default_thr_sched_policy = 0; /* should be default on all platforms */
		}
#if defined (_AIX)
		pthread_attr_setstacksize(&default_thread_attr, 4096*512);
#endif


		ret = pthread_attr_setschedpolicy(&default_thread_attr, default_thr_sched_policy);
		if(ret != 0) {
			rs_strerror_r(ret, errstr, sizeof(errstr));
			fprintf(stderr, "rsyslogd: pthread_attr_setschedpolicy failed during "
				"startup - tried to set priority %d, now using default "
				"priority instead. Error was: %s\n",
				default_thr_sched_policy, errstr);
		}
		ret = pthread_attr_setschedparam(&default_thread_attr, &default_sched_param);
		if(ret != 0) {
			rs_strerror_r(ret, errstr, sizeof(errstr));
			fprintf(stderr, "rsyslogd: pthread_attr_setschedparam failed during "
				"startup - ignored Error was: %s\n", errstr);
		}
		ret = pthread_attr_setinheritsched(&default_thread_attr, PTHREAD_EXPLICIT_SCHED);
		if(ret != 0) {
			rs_strerror_r(ret, errstr, sizeof(errstr));
			fprintf(stderr, "rsyslogd: pthread_attr_setinheritsched failed during "
				"startup - ignoring. Error was: %s\n", errstr);
		}
#endif
		if(ppErrObj != NULL) *ppErrObj = "obj";
		CHKiRet(objClassInit(NULL)); /* *THIS* *MUST* always be the first class initilizer being called! */
		CHKiRet(objGetObjInterface(pObjIF)); /* this provides the root pointer for all other queries */

		/* initialize core classes. We must be very careful with the order of events. Some
		 * classes use others and if we do not initialize them in the right order, we may end
		 * up with an invalid call. The most important thing that can happen is that an error
		 * is detected and needs to be logged, wich in turn requires a broader number of classes
		 * to be available. The solution is that we take care in the order of calls AND use a
		 * class immediately after it is initialized. And, of course, we load those classes
		 * first that we use ourselfs... -- rgerhards, 2008-03-07
		 */
		if(ppErrObj != NULL) *ppErrObj = "statsobj";
		CHKiRet(statsobjClassInit(NULL));
		if(ppErrObj != NULL) *ppErrObj = "prop";
		CHKiRet(propClassInit(NULL));
		if(ppErrObj != NULL) *ppErrObj = "glbl";
		CHKiRet(glblClassInit(NULL));
		if(ppErrObj != NULL) *ppErrObj = "msg";
		CHKiRet(msgClassInit(NULL));
		if(ppErrObj != NULL) *ppErrObj = "ruleset";
		CHKiRet(rulesetClassInit(NULL));
		if(ppErrObj != NULL) *ppErrObj = "wti";
		CHKiRet(wtiClassInit(NULL));
		if(ppErrObj != NULL) *ppErrObj = "wtp";
		CHKiRet(wtpClassInit(NULL));
		if(ppErrObj != NULL) *ppErrObj = "queue";
		CHKiRet(qqueueClassInit(NULL));
		if(ppErrObj != NULL) *ppErrObj = "conf";
		CHKiRet(confClassInit(NULL));
		if(ppErrObj != NULL) *ppErrObj = "parser";
		CHKiRet(parserClassInit(NULL));
		if(ppErrObj != NULL) *ppErrObj = "strgen";
		CHKiRet(strgenClassInit(NULL));
		if(ppErrObj != NULL) *ppErrObj = "rsconf";
		CHKiRet(rsconfClassInit(NULL));
		if(ppErrObj != NULL) *ppErrObj = "lookup";
		CHKiRet(lookupClassInit());
		if(ppErrObj != NULL) *ppErrObj = "dynstats";
		CHKiRet(dynstatsClassInit());

		/* dummy "classes" */
		if(ppErrObj != NULL) *ppErrObj = "str";
		CHKiRet(strInit());
	}

	++iRefCount;
	dbgprintf("rsyslog runtime initialized, version %s, current users %d\n", VERSION, iRefCount);

finalize_it:
	RETiRet;
}


/* globally de-initialze the runtime system
 * NOTE: this is NOT thread safe and must not be called concurrently. If that
 * ever poses a problem, we may use proper mutex calls - not considered needed yet.
 * This function must be provided with the caller's obj object pointer. This is
 * automatically deinitialized by the runtime system.
 * rgerhards, 2008-04-16
 */
rsRetVal
rsrtExit(void)
{
	DEFiRet;

	if(iRefCount == 1) {
		/* do actual de-init only if we are the last runtime user */
		confClassExit();
		glblClassExit();
		rulesetClassExit();
		wtiClassExit();
		wtpClassExit();
		strgenClassExit();
		propClassExit();
		statsobjClassExit();

		objClassExit(); /* *THIS* *MUST/SHOULD?* always be the first class initilizer being
				called (except debug)! */
	}

	--iRefCount;
	/* TODO we must deinit this pointer! pObjIF = NULL; / * no longer exists for this caller */

	dbgprintf("rsyslog runtime de-initialized, current users %d\n", iRefCount);

	RETiRet;
}


/* returns 0 if the rsyslog runtime is not initialized and another value
 * if it is. This function is primarily meant to be used by runtime functions
 * itself. However, it is safe to call it before initializing the runtime.
 * Plugins should NOT rely on this function. The reason is that another caller
 * may have already initialized it but deinits it before this plugin is done.
 * So for plugins and like architectures, the right course of action is to
 * call rsrtInit() and rsrtExit(), which can be called by multiple callers.
 * rgerhards, 2008-04-16
 */
int rsrtIsInit(void)
{
	return iRefCount;
}


/* vim:set ai:
 */
