/* janitor.c - rsyslog's janitor
 *
 * The rsyslog janitor can be used to periodically clean out
 * resources. It was initially developed to close files that
 * were not written to for some time (omfile plugin), but has
 * a generic interface that can be used for all similar tasks.
 *
 * Module begun 2014-05-15 by Rainer Gerhards
 *
 * Copyright (C) 2014-2015 by Rainer Gerhards and Adiscon GmbH.
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

#include "config.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>

#include "rsyslog.h"
#include "janitor.h"

static struct janitorEtry *janitorRoot = NULL; /* TODO: move to runConf? */
static pthread_mutex_t janitorMut = PTHREAD_MUTEX_INITIALIZER;

rsRetVal
janitorAddEtry(void (*cb)(void*), const char *id, void *pUsr)
{
	struct janitorEtry *etry = NULL;
	DEFiRet;
	CHKmalloc(etry = malloc(sizeof(struct janitorEtry)));
	CHKmalloc(etry->id = strdup(id));
	etry->pUsr = pUsr;
	etry->cb = cb;
	etry->next = janitorRoot;
	pthread_mutex_lock(&janitorMut);
	janitorRoot = etry;
	pthread_mutex_unlock(&janitorMut);
	DBGPRINTF("janitor: entry %p, id '%s' added\n", etry, id);
finalize_it:
	if(iRet != RS_RET_OK && etry != NULL)
		free(etry);
	RETiRet;
}

rsRetVal
janitorDelEtry(const char *__restrict__ const id)
{
	struct janitorEtry *curr, *prev = NULL;
	DEFiRet;

	pthread_mutex_lock(&janitorMut);
	for(curr = janitorRoot ; curr != NULL ; curr = curr->next) {
		if(!strcmp(curr->id, id)) {
			if(prev == NULL) {
				janitorRoot = curr->next;
			} else {
				prev->next = curr->next;
			}
			free(curr->id);
			free(curr);
			DBGPRINTF("janitor: deleted entry '%s'\n", id);
			ABORT_FINALIZE(RS_RET_OK);
		}
		prev = curr;
	}
	DBGPRINTF("janitor: to be deleted entry '%s' not found\n", id);
	iRet = RS_RET_NOT_FOUND;
finalize_it:
	pthread_mutex_unlock(&janitorMut);
	RETiRet;
}

/* run the janitor; all entries are processed */
void
janitorRun(void)
{
	struct janitorEtry *curr;

	dbgprintf("janitorRun() called\n");
	pthread_mutex_lock(&janitorMut);
	for(curr = janitorRoot ; curr != NULL ; curr = curr->next) {
		DBGPRINTF("janitor: processing entry %p, id '%s'\n",
			  curr, curr->id);
		curr->cb(curr->pUsr);
	}
	pthread_mutex_unlock(&janitorMut);
}
