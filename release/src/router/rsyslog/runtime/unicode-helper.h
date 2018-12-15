/* This is the header file for unicode support.
 *
 * Currently, this is a dummy module.
 * The following functions are wrappers which hopefully enable us to move
 * from 8-bit chars to unicode with relative ease when we finally attack this
 *
 * Begun 2009-05-21 RGerhards
 *
 * Copyright (C) 2009-2016 by Rainer Gerhards and Adiscon GmbH
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
#ifndef INCLUDED_UNICODE_HELPER_H
#define INCLUDED_UNICODE_HELPER_H

#include <string.h>

#define ustrncpy(psz1, psz2, len) strncpy((char*)(psz1), (char*)(psz2), (len))
#define ustrdup(psz) (uchar*)strdup((char*)(psz))
#define ustrcmp(psz1, psz2) (strcmp((const char*) (psz1), (const char*) (psz2)))
#define ustrlen(psz) (strlen((const char*) (psz)))
#define UCHAR_CONSTANT(x) ((uchar*) (x))
#define CHAR_CONVERT(x) ((char*) (x))

#endif /* multi-include protection */
