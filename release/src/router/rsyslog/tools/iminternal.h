/* Definition of the internal messages input module.
 *
 * Note: we currently do not have an input module spec, but
 * we will have one in the future. This module needs then to be
 * adapted.
 *
 * Copyright 2007 Rainer Gerhards and Adiscon GmbH.
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

#ifndef IMINTERNAL_H_INCLUDED
#define IMINTERNAL_H_INCLUDED
#include "template.h"

/* this is a single entry for a parse routine. It describes exactly
 * one entry point/handler.
 * The short name is cslch (Configfile SysLine CommandHandler)
 */
struct iminternal_s { /* config file sysline parse entry */
	smsg_t *pMsg;	/* the message (in all its glory) */
};
typedef struct iminternal_s iminternal_t;

/* prototypes */
rsRetVal modInitIminternal(void);
rsRetVal modExitIminternal(void);
rsRetVal iminternalAddMsg(smsg_t *pMsg);
rsRetVal iminternalHaveMsgReady(int* pbHaveOne);
rsRetVal iminternalRemoveMsg(smsg_t **ppMsg);

#endif /* #ifndef IMINTERNAL_H_INCLUDED */
