/***********************************************************************
 *
 *  Copyright (c) 2008  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
:>
 *
 ************************************************************************/

#ifndef __CMS_UNICODE_H__
#define __CMS_UNICODE_H__



/*!\file cms_unicode.h
 * \brief Header file for XML Unicode string functions.
 *
 * These functions allow callers to convert strings that contain
 * XML Unicode characters such as &#xhh; or &#dd; to Unicode string.
 *
 */


/** Return true if the given string contains XML Unicode characters
 *
 * @param string (IN) Input string which may contain XML Unicode characters.
 *
 * @return TRUE if the given string contains XML Unicode characters.  The caller
 *              should then call cmsUnicode_unescapeString to do the conversion
 */
UBOOL8 cmsUnicode_isUnescapeNeeded(const char *string);


/** Convert string that contains XML Unicode characters to Unicode string.
 * 
 * @param string        (IN)  Input string which may contain XML Unicode characters.
 * @param unicodedString (OUT) This function will allocate a buffer and put the
 *                            unicoded string in it.  The caller is
 *                            responsible for freeing this buffer.
 *
 * @return CmsRet enum.
 */ 
CmsRet cmsUnicode_unescapeString(const char *string, char **unicodedString);



#endif  /* __CMS_UNICODE_H__ */
