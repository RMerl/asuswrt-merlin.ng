/***********************************************************************
 *
 *  Copyright (c) 2010  Broadcom Corporation
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

#ifndef __CMS_ERRORCODES_H__
#define __CMS_ERRORCODES_H__


/** Given a CmsRet error code, put the associated string in buf.  This is
 *  a multi-thread safe interface.
 *
 *@param ret (IN) CmsRet enum.
 *@param buf (OUT) Buffer to hold the error string.  Buffer is guaranteed
 *                 to be null terminated.  If buffer is too short to hold
 *                 the entire error string, it will be truncated.
 *@param buflen (IN) Length of the buffer.
 *
 *@return pointer to buf.
 */
const char *cmsErr_getStringBuf(CmsRet ret, char *buf, UINT32 buflen);


/** Given a CmsRet error code, return the associated error string.  This
 *  function is not safe for multi-threaded applications since it uses an
 *  internal global buffer to hold the error string.
 *
 *@param ret (IN) CmsRet enum.
 *@return pointer to error string.
 */
const char *cmsErr_getString(CmsRet ret);

#endif /* __CMS_ERRORCODES_H__ */
