/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2002-2005 Apple Computer, Inc. All rights reserved.
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

#ifndef	__MDNS_VXWORKS_H__
#define	__MDNS_VXWORKS_H__

#include	"vxWorks.h"
#include	"config.h"

#include	"semLib.h"

#include	"CommonServices.h"
#include	"DebugServices.h"

#ifdef	__cplusplus
	extern "C" {
#endif

// Forward Declarations

typedef struct	NetworkInterfaceInfoVxWorks		NetworkInterfaceInfoVxWorks;

//---------------------------------------------------------------------------------------------------------------------------
/*!	@struct		SocketSet

	@abstract	Data for IPv4 and IPv6 sockets.
*/

typedef struct	SocketSet	SocketSet;
struct	SocketSet
{
	NetworkInterfaceInfoVxWorks *		info;
	SocketRef							sockV4;
	SocketRef							sockV6;
};

//---------------------------------------------------------------------------------------------------------------------------
/*!	@struct		NetworkInterfaceInfoVxWorks

	@abstract	Interface info for VxWorks.
*/

struct	NetworkInterfaceInfoVxWorks
{
	NetworkInterfaceInfo				ifinfo;		// MUST be the first element in this structure.
	NetworkInterfaceInfoVxWorks *		next;
	mDNSu32								exists;		// 1 = currently exists in getifaddrs list; 0 = doesn't.
													// 2 = exists, but McastTxRx state changed.
	mDNSs32								lastSeen;	// If exists == 0, last time this interface appeared in getifaddrs list.
	mDNSu32								scopeID;	// Interface index / IPv6 scope ID.
	int									family;		// Socket address family of the primary socket.
	mDNSBool							multicast;
	SocketSet							ss;
};

//---------------------------------------------------------------------------------------------------------------------------
/*!	@struct		mDNS_PlatformSupport_struct

	@abstract	Data for mDNS platform plugin.
*/

struct	mDNS_PlatformSupport_struct
{
	NetworkInterfaceInfoVxWorks *		interfaceList;
	SocketSet							unicastSS;
	domainlabel							userNiceLabel;
	domainlabel							userHostLabel;
	
	SEM_ID								lock;
	SEM_ID								initEvent;
	mStatus								initErr;
	SEM_ID								quitEvent;	
	int									commandPipe;
	int									taskID;
	mDNSBool							quit;
};

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	mDNSReconfigure
	
	@abstract	Tell mDNS that the configuration has changed. Call when IP address changes, link goes up after being down, etc.
	
	@discussion
	
	VxWorks does not provide a generic mechanism for getting notified when network interfaces change so this routines
	provides a way for BSP-specific code to signal mDNS that something has changed and it should re-build its interfaces.
*/

void	mDNSReconfigure( void );

//---------------------------------------------------------------------------------------------------------------------------

void	mDNSDeferIPv4( mDNSBool inDefer );

#ifdef	__cplusplus
	}
#endif

#endif	// __MDNS_VXWORKS_H__
