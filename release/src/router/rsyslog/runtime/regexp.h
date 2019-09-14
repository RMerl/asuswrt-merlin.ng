/* The regexp object. It encapsulates the C regexp functionality. The primary
 * purpose of this wrapper class is to enable rsyslogd core to be build without
 * regexp libraries.
 *
 * Copyright 2008-2012 Adiscon GmbH.
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
#ifndef INCLUDED_REGEXP_H
#define INCLUDED_REGEXP_H

#include <regex.h>

/* interfaces */
BEGINinterface(regexp) /* name must also be changed in ENDinterface macro! */
	int (*regcomp)(regex_t *preg, const char *regex, int cflags);
	int (*regexec)(const regex_t *preg, const char *string, size_t nmatch, regmatch_t pmatch[], int eflags);
	size_t (*regerror)(int errcode, const regex_t *preg, char *errbuf, size_t errbuf_size);
	void (*regfree)(regex_t *preg);
ENDinterface(regexp)
#define regexpCURR_IF_VERSION 1 /* increment whenever you change the interface structure! */


/* prototypes */
PROTOTYPEObj(regexp);

/* the name of our library binary */
#define LM_REGEXP_FILENAME "lmregexp"

#endif /* #ifndef INCLUDED_REGEXP_H */
