/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 1997-2004 Apple Computer, Inc. All rights reserved.
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

#include "WinServices.h"
#include <DebugServices.h>


//===========================================================================================================================
//	UTF8StringToStringObject
//===========================================================================================================================

OSStatus	UTF8StringToStringObject( const char *inUTF8, CString &inObject )
{
	OSStatus		err;
	int				n;
	BSTR			unicode;
	
	unicode = NULL;
	
	n = MultiByteToWideChar( CP_UTF8, 0, inUTF8, -1, NULL, 0 );
	if( n > 0 )
	{
		unicode = (BSTR) malloc( (size_t)( n * sizeof( wchar_t ) ) );
		if( !unicode )
		{
			err = ERROR_INSUFFICIENT_BUFFER;
			goto exit;
		}

		n = MultiByteToWideChar( CP_UTF8, 0, inUTF8, -1, unicode, n );
		try
		{
			inObject = unicode;
		}
		catch( ... )
		{
			err = ERROR_NO_UNICODE_TRANSLATION;
			goto exit;
		}
	}
	else
	{
		inObject = "";
	}
	err = ERROR_SUCCESS;
	
exit:
	if( unicode )
	{
		free( unicode );
	}
	return( err );
}


//===========================================================================================================================
//	UTF8StringToStringObject
//===========================================================================================================================

OSStatus
StringObjectToUTF8String( CString &inObject, char* outUTF8, size_t outUTF8Len )
{
    OSStatus err = kNoErr;

	memset( outUTF8, 0, outUTF8Len );

	if ( inObject.GetLength() > 0 )
    {
		size_t size;

		size = (size_t) WideCharToMultiByte( CP_UTF8, 0, inObject.GetBuffer(), inObject.GetLength(), outUTF8, (int) outUTF8Len, NULL, NULL);
        err = translate_errno( size != 0, GetLastError(), kUnknownErr );
        require_noerr( err, exit );
    }

exit:

	return err;
}
