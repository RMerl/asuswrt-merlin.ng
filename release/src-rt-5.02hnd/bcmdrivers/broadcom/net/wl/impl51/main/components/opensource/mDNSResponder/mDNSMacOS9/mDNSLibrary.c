/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
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

// Define the required CFM Shared Library entry and exit points
#include <CodeFragments.h>
#include "mDNSEmbeddedAPI.h"			// Defines the interface to the client layer above
#include "mDNSMacOS9.h"					// Defines the specific types needed to run mDNS on this platform

mDNS mDNSStorage;
static mDNS_PlatformSupport PlatformSupportStorage;
// Start off with a default cache of 16K (about 100 records)
#define RR_CACHE_SIZE ((16*1024) / sizeof(CacheRecord))
static CacheEntity rrcachestorage[RR_CACHE_SIZE];

mDNSlocal void mDNS_StatusCallback(mDNS *const m, mStatus result)
	{
	if (result == mStatus_GrowCache)
		{
		// Allocate another chunk of cache storage
		CacheEntity *storage = OTAllocMem(sizeof(CacheEntity) * RR_CACHE_SIZE);
		if (storage) mDNS_GrowCache(m, storage, RR_CACHE_SIZE);
		}
	}

extern pascal OSErr mDNS_CFMInit(const CFragInitBlock *theInitBlock);
pascal OSErr mDNS_CFMInit(const CFragInitBlock *theInitBlock)
	{
	extern pascal OSErr __initialize(const CFragInitBlock *theInitBlock);
	__initialize(theInitBlock);	// MUST do this first!
		{
		mStatus err;
		THz oldZone = GetZone();
		SetZone(SystemZone());
		LogMsg("mDNS/DNS-SD with Macsbug breaks -- do not ship this version to customers");
		err = mDNS_Init(&mDNSStorage, &PlatformSupportStorage, rrcachestorage, RR_CACHE_SIZE,
			mDNS_Init_AdvertiseLocalAddresses, mDNS_StatusCallback, mDNS_Init_NoInitCallbackContext);
		SetZone(oldZone);
		return((OSErr)err);
		}
	}
	
extern void mDNS_CFMTerm(void);
void mDNS_CFMTerm(void)
	{
	extern pascal void  __terminate(void);
	LogMsg("mDNS_CFMTerm");
	mDNS_Close(&mDNSStorage);
	__terminate();
	}
