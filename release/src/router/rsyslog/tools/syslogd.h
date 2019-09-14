/* common header for syslogd
 * Copyright 2007-2012 Adiscon GmbH.
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
#ifndef	SYSLOGD_H_INCLUDED
#define	SYSLOGD_H_INCLUDED 1

#include "syslogd-types.h"
#include "objomsr.h"
#include "modules.h"
#include "template.h"
#include "action.h"
#include "linkedlist.h"

/* the following prototypes should go away once we have an input
 * module interface -- rgerhards, 2007-12-12
 */
extern int NoHops;
extern int send_to_all;
extern int Debug;
#include "dirty.h"

#endif /* #ifndef SYSLOGD_H_INCLUDED */
