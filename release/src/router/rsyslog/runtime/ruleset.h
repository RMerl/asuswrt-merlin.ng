/* The ruleset object.
 *
 * This implements rulesets within rsyslog.
 *
 * Copyright 2009-2013 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of the rsyslog runtime library.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *       -or-
 *       see COPYING.ASL20 in the source distribution
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef INCLUDED_RULESET_H
#define INCLUDED_RULESET_H

#include "queue.h"
#include "linkedlist.h"
#include "rsconf.h"

/* the ruleset object */
struct ruleset_s {
	BEGINobjInstance;	/* Data to implement generic object - MUST be the first data element! */
	uchar *pszName;		/* name of our ruleset */
	qqueue_t *pQueue;	/* "main" message queue, if the ruleset has its own (else NULL) */
	struct cnfstmt *root;
	struct cnfstmt *last;
	parserList_t *pParserLst;/* list of parsers to use for this ruleset */
};

/* interfaces */
BEGINinterface(ruleset) /* name must also be changed in ENDinterface macro! */
	INTERFACEObjDebugPrint(ruleset);
	rsRetVal (*DebugPrintAll)(rsconf_t *conf);
	rsRetVal (*Construct)(ruleset_t **ppThis);
	rsRetVal (*ConstructFinalize)(rsconf_t *conf, ruleset_t __attribute__((unused)) *pThis);
	rsRetVal (*Destruct)(ruleset_t **ppThis);
	rsRetVal (*DestructAllActions)(rsconf_t *conf);
	rsRetVal (*SetName)(ruleset_t *pThis, uchar *pszName);
	rsRetVal (*ProcessBatch)(batch_t*, wti_t *);
	rsRetVal (*GetRuleset)(rsconf_t *conf, ruleset_t **ppThis, uchar*);
	rsRetVal (*SetDefaultRuleset)(rsconf_t *conf, uchar*);
	rsRetVal (*SetCurrRuleset)(rsconf_t *conf, uchar*);
	ruleset_t* (*GetCurrent)(rsconf_t *conf);
	qqueue_t* (*GetRulesetQueue)(ruleset_t*);
	/* v3, 2009-11-04 */
	parserList_t* (*GetParserList)(rsconf_t *conf, smsg_t *);
	/* v5, 2011-04-19
	 * added support for the rsconf object -- fundamental change
	 * v6, 2011-07-15
	 * removed conf ptr from SetName, AddRule as the flex/bison based
	 * system uses globals in any case.
	 */
	/* v7, 2012-09-04 */
	/* AddRule() removed */
	/*TODO:REMOVE*/rsRetVal (*IterateAllActions)(rsconf_t *conf, rsRetVal (*pFunc)(void*, void*), void* pParam);
	void (*AddScript)(ruleset_t *pThis, struct cnfstmt *script);
	/* v8: changed processBatch interface */
ENDinterface(ruleset)
#define rulesetCURR_IF_VERSION 8 /* increment whenever you change the interface structure! */


/* prototypes */
PROTOTYPEObj(ruleset);

/* TODO: remove these -- currently done dirty for config file
 * redo -- rgerhards, 2011-04-19
 * rgerhards, 2012-04-19: actually, it may be way cooler not to remove
 * them and use plain c-style conventions at least inside core objects.
 */
rsRetVal rulesetDestructForLinkedList(void *pData);
rsRetVal rulesetKeyDestruct(void __attribute__((unused)) *pData);

/* Get name associated to ruleset. This function cannot fail (except,
 * of course, if previously something went really wrong). Returned
 * pointer is read-only.
 * rgerhards, 2012-04-18
 */
#define rulesetGetName(pRuleset) ((pRuleset)->pszName)

/* returns 1 if the ruleset has a queue associtated, 0 if not */
#define rulesetHasQueue(pRuleset) ((pRuleset)->pQueue == NULL ? 0 : 1)


/* we will most probably convert this module back to traditional C
 * calling sequence, so here we go...
 */
rsRetVal rulesetGetRuleset(rsconf_t *conf, ruleset_t **ppRuleset, uchar *pszName);
rsRetVal rulesetOptimizeAll(rsconf_t *conf);
rsRetVal rulesetProcessCnf(struct cnfobj *o);
rsRetVal activateRulesetQueues(void);

/* Set a current rule set to already-known pointer */
#define rulesetSetCurrRulesetPtr(pRuleset) (loadConf->rulesets.pCurr = (pRuleset))

#endif /* #ifndef INCLUDED_RULESET_H */
