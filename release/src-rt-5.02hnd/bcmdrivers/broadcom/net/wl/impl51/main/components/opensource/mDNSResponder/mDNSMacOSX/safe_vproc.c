/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2009 Apple Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <assert.h>
#include <vproc.h>
#include "safe_vproc.h"
#include "mDNSDebug.h"
#include <TargetConditionals.h>

#if defined(VPROC_HAS_TRANSACTIONS) && !TARGET_OS_EMBEDDED

static vproc_transaction_t transaction = NULL;

void safe_vproc_transaction_begin(void)
	{
	if (vproc_transaction_begin)
		{
		if (transaction) { LogMsg("safe_vproc_transaction_begin: Already have a transaction"); }
		else transaction = vproc_transaction_begin(NULL);
		}
	}
	
void safe_vproc_transaction_end(void)
	{
	if (vproc_transaction_end)
		{
		if (transaction) { vproc_transaction_end(NULL, transaction); transaction = NULL; }
		else LogMsg("safe_vproc_transaction_end: No current transaction");
		}
	}

#else

#if ! TARGET_OS_EMBEDDED
#include <stdio.h>
#include <CoreFoundation/CFString.h>

CF_EXPORT CFDictionaryRef _CFCopySystemVersionDictionary(void);
CF_EXPORT const CFStringRef _kCFSystemVersionBuildVersionKey;
#define OSXVers_10_6_SnowLeopard 10

void safe_vproc_transaction_begin(void)
	{
	static int majorversion = 0;
	if (!majorversion)
		{
		CFDictionaryRef vers = _CFCopySystemVersionDictionary();
		if (vers)
			{
			char buildver[256];
			CFStringRef cfbuildver = CFDictionaryGetValue(vers, _kCFSystemVersionBuildVersionKey);
			if (cfbuildver && CFStringGetCString(cfbuildver, buildver, sizeof(buildver), kCFStringEncodingUTF8))
				sscanf(buildver, "%d", &majorversion);
			CFRelease(vers);
			}
		if (!majorversion || majorversion >= OSXVers_10_6_SnowLeopard)
			LogMsg("Compiled without vproc_transaction support");
		}
	}

#else

void safe_vproc_transaction_begin(void) { }

#endif

void safe_vproc_transaction_end(void) { }

#endif
