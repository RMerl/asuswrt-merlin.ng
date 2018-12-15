/* The rsconf object. It models a complete rsyslog configuration.
 *
 * Copyright 2011-2016 Rainer Gerhards and Adiscon GmbH.
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
#ifndef INCLUDED_RSCONF_H
#define INCLUDED_RSCONF_H

#include "linkedlist.h"
#include "queue.h"
#include "lookup.h"
#include "dynstats.h"

/* --- configuration objects (the plan is to have ALL upper layers in this file) --- */

/* queue config parameters. TODO: move to queue.c? */
struct queuecnf_s {
	int iMainMsgQueueSize;		/* size of the main message queue above */
	int iMainMsgQHighWtrMark;	/* high water mark for disk-assisted queues */
	int iMainMsgQLowWtrMark;	/* low water mark for disk-assisted queues */
	int iMainMsgQDiscardMark;	/* begin to discard messages */
	int iMainMsgQDiscardSeverity;	/* by default, discard nothing to prevent unintentional loss */
	int iMainMsgQueueNumWorkers;	/* number of worker threads for the mm queue above */
	queueType_t MainMsgQueType;	/* type of the main message queue above */
	uchar *pszMainMsgQFName;	/* prefix for the main message queue file */
	int64 iMainMsgQueMaxFileSize;
	int iMainMsgQPersistUpdCnt;	/* persist queue info every n updates */
	int bMainMsgQSyncQeueFiles;	/* sync queue files on every write? */
	int iMainMsgQtoQShutdown;	/* queue shutdown (ms) */
	int iMainMsgQtoActShutdown;	/* action shutdown (in phase 2) */
	int iMainMsgQtoEnq;		/* timeout for queue enque */
	int iMainMsgQtoWrkShutdown;	/* timeout for worker thread shutdown */
	int iMainMsgQWrkMinMsgs;	/* minimum messages per worker needed to start a new one */
	int iMainMsgQDeqSlowdown;	/* dequeue slowdown (simple rate limiting) */
	int64 iMainMsgQueMaxDiskSpace;	/* max disk space allocated 0 ==> unlimited */
	int64 iMainMsgQueDeqBatchSize;	/* dequeue batch size */
	int bMainMsgQSaveOnShutdown;	/* save queue on shutdown (when DA enabled)? */
	int iMainMsgQueueDeqtWinFromHr;	/* hour begin of time frame when queue is to be dequeued */
	int iMainMsgQueueDeqtWinToHr;	/* hour begin of time frame when queue is to be dequeued */
};

/* globals are data items that are really global, and can be set only
 * once (at least in theory, because the legacy system permits them to
 * be re-set as often as the user likes).
 */
struct globals_s {
	int bDebugPrintTemplateList;
	int bDebugPrintModuleList;
	int bDebugPrintCfSysLineHandlerList;
	int bLogStatusMsgs;	/* log rsyslog start/stop/HUP messages? */
	int bErrMsgToStderr;	/* print error messages to stderr
				  (in addition to everything else)? */
	int maxErrMsgToStderr;	/* how many messages to forward at most to stderr? */
	int bAbortOnUncleanConfig; /* abort run (rather than starting with partial
				      config) if there was any issue in conf */
	int uidDropPriv;	/* user-id to which priveleges should be dropped to */
	int gidDropPriv;	/* group-id to which priveleges should be dropped to */
	int gidDropPrivKeepSupplemental; /* keep supplemental groups when dropping? */
	int umask;		/* umask to use */
	uchar *pszConfDAGFile;	/* name of config DAG file, non-NULL means generate one */

	// TODO are the following ones defaults?
	int bReduceRepeatMsgs; /* reduce repeated message - 0 - no, 1 - yes */

	//TODO: other representation for main queue? Or just load it differently?
	queuecnf_t mainQ;	/* main queue parameters */
};

/* (global) defaults are global in the sense that they are accessible
 * to all code, but they can change value and other objects (like
 * actions) actually copy the value a global had at the time the action
 * was defined. In that sense, a global default is just that, a default,
 * wich can (and will) be changed in the course of config file
 * processing. Once the config file has been processed, defaults
 * can be dropped. The current code does not do this for simplicity.
 * That is not a problem, because the defaults do not take up much memory.
 * At a later stage, we may think about dropping them. -- rgerhards, 2011-04-19
 */
struct defaults_s {
	int remove_me_when_first_real_member_is_added;
};


/* list of modules loaded in this configuration (config specific module list) */
struct cfgmodules_etry_s {
	cfgmodules_etry_t *next;
	modInfo_t *pMod;
	void *modCnf;		/* pointer to the input module conf */
	/* the following data is input module specific */
	sbool canActivate;	/* OK to activate this config? */
	sbool canRun;		/* OK to run this config? */
};

struct cfgmodules_s {
	cfgmodules_etry_t *root;
};

/* outchannel-specific data */
struct outchannels_s {
	struct outchannel *ochRoot;	/* the root of the outchannel list */
	struct outchannel *ochLast;	/* points to the last element of the outchannel list */
};

struct templates_s {
	struct template *root;	/* the root of the template list */
	struct template *last;	/* points to the last element of the template list */
	struct template *lastStatic; /* last static element of the template list */
};


struct actions_s {
	unsigned nbrActions;		/* number of actions */
};


struct rulesets_s {
	linkedList_t llRulesets; /* this is NOT a pointer - no typo here ;) */

	/* support for legacy rsyslog.conf format */
	ruleset_t *pCurr; /* currently "active" ruleset */
	ruleset_t *pDflt; /* current default ruleset, e.g. for binding to actions which have no other */
};


/* --- end configuration objects --- */

/* the rsconf object */
struct rsconf_s {
	BEGINobjInstance;	/* Data to implement generic object - MUST be the first data element! */
	cfgmodules_t modules;
	globals_t globals;
	defaults_t defaults;
	templates_t templates;
	lookup_tables_t lu_tabs;
	dynstats_buckets_t dynstats_buckets;
	outchannels_t och;
	actions_t actions;
	rulesets_t rulesets;
	/* note: rulesets include the complete output part:
	 *  - rules
	 *  - filter (as part of the action)
	 *  - actions
	 * Of course, we need to debate if we shall change that some time...
	 */
};


/* interfaces */
BEGINinterface(rsconf) /* name must also be changed in ENDinterface macro! */
	INTERFACEObjDebugPrint(rsconf);
	rsRetVal (*Destruct)(rsconf_t **ppThis);
	rsRetVal (*Load)(rsconf_t **ppThis, uchar *confFile);
	rsRetVal (*Activate)(rsconf_t *ppThis);
ENDinterface(rsconf)
// TODO: switch version to 1 for first "complete" version!!!! 2011-04-20
#define rsconfCURR_IF_VERSION 0 /* increment whenever you change the interface above! */


/* prototypes */
PROTOTYPEObj(rsconf);

/* globally-visible external data */
extern rsconf_t *runConf;/* the currently running config */
extern rsconf_t *loadConf;/* the config currently being loaded (no concurrent config load supported!) */


int rsconfNeedDropPriv(rsconf_t *const cnf);

/* some defaults (to be removed?) */
#define DFLT_bLogStatusMsgs 1

#endif /* #ifndef INCLUDED_RSCONF_H */
