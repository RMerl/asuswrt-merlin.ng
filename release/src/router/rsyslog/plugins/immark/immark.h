/* immark.h
 * These are the definitions for the built-in mark message generation module. This
 * file may disappear when this has become a loadable module.
 *
 * File begun on 2007-12-12 by RGerhards (extracted from syslogd.c)
 *
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
#ifndef	IMMARK_H_INCLUDED
#define	IMMARK_H_INCLUDED 1

/* prototypes */
rsRetVal immark_runInput(void);

#endif /* #ifndef IMMARK_H_INCLUDED */
/*
 * vi:set ai:
 */
