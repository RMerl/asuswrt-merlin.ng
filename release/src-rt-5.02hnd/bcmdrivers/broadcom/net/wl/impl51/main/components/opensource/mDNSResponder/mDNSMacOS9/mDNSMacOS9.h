/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2002-2003 Apple Computer, Inc. All rights reserved.
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

// ***************************************************************************
// Classic Mac (Open Transport) structures

//#include <Files.h>	// OpenTransport.h requires this
#include <OpenTransport.h>
#include <OpenTptInternet.h>
#include <OpenTptClient.h>

typedef enum
	{
	mOT_Closed = 0,		// We got kOTProviderIsClosed message
	mOT_Reset,			// We got xOTStackWasLoaded message
	mOT_Start,			// We've called OTAsyncOpenEndpoint
	mOT_ReusePort,		// Have just done kReusePortOption
	mOT_RcvDestAddr,	// Have just done kRcvDestAddrOption
	mOT_SetUTTL,		// Have just done kSetUnicastTTLOption
	mOT_SetMTTL,		// Have just done kSetMulticastTTLOption
	mOT_LLScope,		// Have just done kAddLinkMulticastOption
//	mOT_AdminScope,		// Have just done kAddAdminMulticastOption
	mOT_Bind,			// We've just called OTBind
	mOT_Ready			// Got T_BINDCOMPLETE; Interface is registered and active
	} mOT_State;

typedef struct { TOptionHeader h; mDNSv4Addr multicastGroupAddress; mDNSv4Addr InterfaceAddress; } TIPAddMulticastOption;
typedef struct { TOptionHeader h; UInt8 val; } TSetByteOption;
typedef struct { TOptionHeader h; UInt32 flag; } TSetBooleanOption;

// TOptionBlock is a union of various types.
// What they all have in common is that they all start with a TOptionHeader.
typedef union  { TOptionHeader h; TIPAddMulticastOption m; TSetByteOption i; TSetBooleanOption b; } TOptionBlock;

struct mDNS_PlatformSupport_struct
	{
	EndpointRef ep;
	UInt32 mOTstate;				// mOT_State enum
	TOptionBlock optBlock;
	TOptMgmt optReq;
	long OTTimerTask;
	UInt32 nesting;
	NetworkInterfaceInfo interface;
	};
