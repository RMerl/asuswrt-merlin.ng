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

	Contains:	mDNS platform plugin for VxWorks.

	Copyright:  Copyright (C) 2002-2003 Apple Computer, Inc., All Rights Reserved.

 */

#ifndef	__MDNS_VXWORKS__
#define	__MDNS_VXWORKS__

#include	"vxWorks.h"
#include	"semLib.h"

#include	"mDNSEmbeddedAPI.h"

#ifdef	__cplusplus
	extern "C" {
#endif

// Forward Declarations

typedef struct	MDNSInterfaceItem	MDNSInterfaceItem;

//---------------------------------------------------------------------------------------------------------------------------
/*!	@struct		mDNS_PlatformSupport_struct

	@abstract	Structure containing platform-specific data.
*/

struct	mDNS_PlatformSupport_struct
{
	SEM_ID					lockID;
	SEM_ID					readyEvent;
	mStatus					taskInitErr;
	SEM_ID					quitEvent;
	MDNSInterfaceItem *		interfaceList;
	int						commandPipe;
	int						task;
	mDNSBool				quit;
	long					configID;
	int						rescheduled;
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
/*!	@struct		ifaddrs

	@abstract	Interface information
*/

struct ifaddrs
{
	struct ifaddrs *	ifa_next;
	char *				ifa_name;
	u_int				ifa_flags;
	struct sockaddr	*	ifa_addr;
	struct sockaddr	*	ifa_netmask;
	struct sockaddr	*	ifa_dstaddr;
	void *				ifa_data;
};

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	getifaddrs

	@abstract	Builds a linked list of interfaces. Caller must free using freeifaddrs if successful.
*/

int	getifaddrs( struct ifaddrs **outAddrs );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	freeifaddrs

	@abstract	Frees a linked list of interfaces built with getifaddrs.
*/

void	freeifaddrs( struct ifaddrs *inAddrs );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	sock_pton

	@abstract	Converts a 'p'resentation address string into a 'n'umeric sockaddr structure.
	
	@result		0 if successful or an error code on failure.
*/

int	sock_pton( const char *inString, int inFamily, void *outAddr, size_t inAddrSize, size_t *outAddrSize );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	sock_ntop

	@abstract	Converts a 'n'umeric sockaddr structure into a 'p'resentation address string.
	
	@result		Ptr to 'p'resentation address string buffer if successful or NULL on failure.
*/

char *	sock_ntop( const void *inAddr, size_t inAddrSize, char *inBuffer, size_t inBufferSize );

#ifdef	__cplusplus
	}
#endif

#endif	// __MDNS_VXWORKS__
