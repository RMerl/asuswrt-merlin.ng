/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2002-2007 Apple Inc. All rights reserved.
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

    
#pragma once

#include "stdafx.h"

extern HINSTANCE	GetNonLocalizedResources();
extern HINSTANCE	GetLocalizedResources();

//-------------------------------------------------
//	CCPApp
//-------------------------------------------------

class CCPApp : public CWinApp
{
public:

	CCPApp();
	virtual ~CCPApp();

protected:

	virtual BOOL    InitInstance();

	void
	Register( LPCTSTR inClsidString, LPCTSTR inName, LPCTSTR inCanonicalName, LPCTSTR inCategory, LPCTSTR inLocalizedName, LPCTSTR inInfoTip, LPCTSTR inIconPath, LPCTSTR inExePath );

	void
	Unregister( LPCTSTR clsidString );

	DECLARE_DYNAMIC(CCPApp);
};
