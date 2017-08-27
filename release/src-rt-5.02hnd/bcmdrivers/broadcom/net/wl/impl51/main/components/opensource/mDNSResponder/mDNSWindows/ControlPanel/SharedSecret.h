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

    
#pragma once

#include "resource.h"


//---------------------------------------------------------------------------------------------------------------------------
//	CSharedSecret
//---------------------------------------------------------------------------------------------------------------------------

class CSharedSecret : public CDialog
{
	DECLARE_DYNAMIC(CSharedSecret)

public:
	CSharedSecret(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSharedSecret();

// Dialog Data
	enum { IDD = IDR_SECRET };

	void
	Load( CString zone );

	void
	Commit( CString zone );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:

	CString m_key;
	CString m_secret;
};
