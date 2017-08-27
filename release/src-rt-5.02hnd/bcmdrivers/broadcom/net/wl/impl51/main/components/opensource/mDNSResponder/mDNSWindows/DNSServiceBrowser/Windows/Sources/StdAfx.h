/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2002-2004 Apple Computer, Inc. All rights reserved.
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

#if !defined(AFX_STDAFX_H__424305D2_0A97_4AA0_B9B1_A7D90D18EBA0__INCLUDED_)
#define AFX_STDAFX_H__424305D2_0A97_4AA0_B9B1_A7D90D18EBA0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#ifndef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
	#define WINVER 0x0400	// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#include	<afxwin.h>		// MFC core and standard components
#include	<afxext.h>		// MFC extensions
#include	<afxdtctl.h>	// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
	#include	<afxcmn.h>	// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include	<winsock2.h>

#include	<stdlib.h>

#include	"DNSServices.h"

#include	"Application.h"

#include	"ChooserDialog.h"

#endif // !defined(AFX_STDAFX_H__424305D2_0A97_4AA0_B9B1_A7D90D18EBA0__INCLUDED_)
