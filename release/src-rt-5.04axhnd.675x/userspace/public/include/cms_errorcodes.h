/***********************************************************************
 *
 *  Copyright (c) 2010  Broadcom Corporation
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
