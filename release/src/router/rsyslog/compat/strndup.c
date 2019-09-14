/* compatibility file for systems without strndup.
 *
 * Copyright 2015 Rainer Gerhards and Adiscon
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
#include "config.h"
#ifndef HAVE_STRNDUP

#include <stdlib.h>
#include <string.h>
char *
strndup(const char *s, size_t n)
{
	const size_t len = strlen(s);
	if(len <= n)
		return strdup(s);
	char *const new_s = malloc(n+1);
	if(new_s == NULL)
		return NULL;
	memcpy(new_s, s, n);
	new_s[n] = '\0';
	return new_s;
}

#endif /* #ifndef HAVE_STRNDUP */
