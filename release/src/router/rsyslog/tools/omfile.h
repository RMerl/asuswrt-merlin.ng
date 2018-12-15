/* omfile.h
 * These are the definitions for the build-in file output module.
 *
 * File begun on 2007-07-21 by RGerhards
 *
 * Copyright 2007-2012 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of rsyslog.
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
#ifndef	OMFILE_H_INCLUDED
#define	OMFILE_H_INCLUDED 1

/* prototypes */
rsRetVal modInitFile(int iIFVersRequested __attribute__((unused)), int *ipIFVersProvided, rsRetVal (**pQueryEtryPt)(),
	rsRetVal (*pHostQueryEtryPt)(uchar*, rsRetVal (**)()), modInfo_t*);

/* the define below is dirty, but we need it for ompipe integration. There is no
 * other way to have the functionality (well, one way would be to go through the
 * globals, but that seems not yet justified. -- rgerhards, 2010-03-01
 */
uchar	*pszFileDfltTplName;
#endif /* #ifndef OMFILE_H_INCLUDED */
/* vi:set ai:
 */
