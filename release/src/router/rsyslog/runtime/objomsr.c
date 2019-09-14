/* objomsr.c
 * Implementation of the omsr (omodStringRequest) object.
 *
 * File begun on 2007-07-27 by RGerhards
 *
 * Copyright 2007-2012 Rainer Gerhards and Adiscon GmbH.
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "rsyslog.h"
#include "objomsr.h"


/* destructor
 */
rsRetVal OMSRdestruct(omodStringRequest_t *pThis)
{
	int i;

	assert(pThis != NULL);
	/* free the strings */
	if(pThis->ppTplName != NULL) {
		for(i = 0 ; i < pThis->iNumEntries ; ++i) {
			free(pThis->ppTplName[i]);
		}
		free(pThis->ppTplName);
	}
	if(pThis->piTplOpts != NULL)
		free(pThis->piTplOpts);
	free(pThis);
	
	return RS_RET_OK;
}


/* constructor
 */
rsRetVal OMSRconstruct(omodStringRequest_t **ppThis, int iNumEntries)
{
	omodStringRequest_t *pThis = NULL;
	DEFiRet;

	assert(ppThis != NULL);
	assert(iNumEntries >= 0);
	if(iNumEntries > CONF_OMOD_NUMSTRINGS_MAXSIZE) {
		ABORT_FINALIZE(RS_RET_MAX_OMSR_REACHED);
	}
	CHKmalloc(pThis = calloc(1, sizeof(omodStringRequest_t)));

	/* got the structure, so fill it */
	if(iNumEntries > 0) {
		pThis->iNumEntries = iNumEntries;
		/* allocate string for template name array. The individual strings will be
		 * allocated as the code progresses (we do not yet know the string sizes)
		 */
		CHKmalloc(pThis->ppTplName = calloc(iNumEntries, sizeof(uchar*)));

	/* allocate the template options array. */
		CHKmalloc(pThis->piTplOpts = calloc(iNumEntries, sizeof(int)));
	}
	
finalize_it:
	if(iRet != RS_RET_OK) {
		if(pThis != NULL) {
			OMSRdestruct(pThis);
			pThis = NULL;
		}
	}
	*ppThis = pThis;
	RETiRet;
}

/* set a template name and option to the object. Index must be given. The pTplName must be
 * pointing to memory that can be freed. If in doubt, the caller must strdup() the value.
 */
rsRetVal OMSRsetEntry(omodStringRequest_t *pThis, int iEntry, uchar *pTplName, int iTplOpts)
{
	assert(pThis != NULL);
	assert(iEntry < pThis->iNumEntries);

	if(pThis->ppTplName[iEntry] != NULL)
		free(pThis->ppTplName[iEntry]);
	pThis->ppTplName[iEntry] = pTplName;
	pThis->piTplOpts[iEntry] = iTplOpts;

	return RS_RET_OK;
}


/* get number of entries for this object
 */
int OMSRgetEntryCount(omodStringRequest_t *pThis)
{
	assert(pThis != NULL);
	return pThis->iNumEntries;
}


/* return data for a specific entry. All data returned is
 * read-only and lasts only as long as the object lives. If the caller
 * needs it for an extended period of time, the caller must copy the
 * strings. Please note that the string pointer may be NULL, which is the
 * case when it was never set.
 */
int OMSRgetEntry(omodStringRequest_t *pThis, int iEntry, uchar **ppTplName, int *piTplOpts)
{
	assert(pThis != NULL);
	assert(ppTplName != NULL);
	assert(piTplOpts != NULL);
	assert(iEntry < pThis->iNumEntries);

	*ppTplName = pThis->ppTplName[iEntry];
	*piTplOpts = pThis->piTplOpts[iEntry];

	return RS_RET_OK;
}


/* return the full set of template options that are supported by this version of
 * OMSR. They are returned in an unsigned long value. The caller can mask that
 * value to check on the option he is interested in.
 * Note that this interface was added in 4.1.6, so a plugin must obtain a pointer
 * to this interface via queryHostEtryPt().
 * rgerhards, 2009-04-03
 */
rsRetVal
OMSRgetSupportedTplOpts(unsigned long *pOpts)
{
	DEFiRet;
	assert(pOpts != NULL);
	*pOpts = OMSR_RQD_TPL_OPT_SQL | OMSR_TPL_AS_ARRAY | OMSR_TPL_AS_MSG
		 | OMSR_TPL_AS_JSON;
	RETiRet;
}

/* vim:set ai:
 */
