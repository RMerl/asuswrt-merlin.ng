/***********************************************************************
 *
 *  Copyright (c) 2008  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
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
