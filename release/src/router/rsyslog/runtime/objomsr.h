/* Definition of the omsr (omodStringRequest) object.
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

#ifndef OBJOMSR_H_INCLUDED
#define OBJOMSR_H_INCLUDED

/* define flags for required template options */
#define OMSR_NO_RQD_TPL_OPTS	0
#define OMSR_RQD_TPL_OPT_SQL	1
/* only one of OMSR_TPL_AS_ARRAY, _AS_MSG, or _AS_JSON  must be specified,
 * if all are given results are unpredictable.
 */
#define OMSR_TPL_AS_ARRAY	2	 /* introduced in 4.1.6, 2009-04-03 */
#define OMSR_TPL_AS_MSG		4	 /* introduced in 5.3.4, 2009-11-02 */
#define OMSR_TPL_AS_JSON	8	 /* introduced in 6.5.1, 2012-09-02 */
/* next option is 16, 32, 64, ... */

struct omodStringRequest_s {	/* strings requested by output module for doAction() */
	int iNumEntries;	/* number of array entries for data elements below */
	uchar **ppTplName;	/* pointer to array of template names */
	int *piTplOpts;/* pointer to array of check-options when pulling template */
};
typedef struct omodStringRequest_s omodStringRequest_t;

/* prototypes */
rsRetVal OMSRdestruct(omodStringRequest_t *pThis);
rsRetVal OMSRconstruct(omodStringRequest_t **ppThis, int iNumEntries);
rsRetVal OMSRsetEntry(omodStringRequest_t *pThis, int iEntry, uchar *pTplName, int iTplOpts);
rsRetVal OMSRgetSupportedTplOpts(unsigned long *pOpts);
int OMSRgetEntryCount(omodStringRequest_t *pThis);
int OMSRgetEntry(omodStringRequest_t *pThis, int iEntry, uchar **ppTplName, int *piTplOpts);

#endif /* #ifndef OBJOMSR_H_INCLUDED */
