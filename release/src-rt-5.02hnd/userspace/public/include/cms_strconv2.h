/***********************************************************************
 *
 *  Copyright (c) 2013-2014  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2013:DUAL/GPL:standard
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

#ifndef __CMS_STRCONV2_H__
#define __CMS_STRCONV2_H__

/*!\file cms_strconv2.h
 * \brief Header file for the application to convert value from string to other formats.
 * This is in the cms_util library.
 */

/** convert vpiVciStr (eg. "0/35") to vpi=2 and vci=35 (TR181 format)
 * @param vpiVciStr (IN) vpi/vci string
 * @param vpi (OUT) vpi in SINT32 format 
 * @param vci (OUT) vpi in SINT32 format
 * @return CmsRet enum.
 */
CmsRet cmsUtl_atmVpiVciStrToNum_dev2(const char *vpiVciStr, SINT32 *vpi, SINT32 *vci);

/**
 * @param vpiVciStr (IN) prefix:vpi/vci string where prefix is PVC:vpi/vci
 * @param vpi (OUT) vpi in SINT32 format 
 * @param vci (OUT) vpi in SINT32 format
 * @return CmsRet enum.
 */
CmsRet cmsUtl_atmVpiVciNumToStr_dev2(const SINT32 vpi, const SINT32 vci, char *vpiVciStr);



/** Add a fullpath string to a comma separated list of fullpath strings
 *
 */
CmsRet cmsUtl_addFullPathToCSL(const char *fullPath, char *CSLBuf, UINT32 CSLlen);


/** Delete a fullpath string from a comma separated list of fullpath strings
 *
 */
void cmsUtl_deleteFullPathFromCSL(const char *fullPath, char *CSLBuf);


/** Return TRUE if the given fullpath string is in the comma separated list
 *  of fullpath string.
 *
 */
UBOOL8 cmsUtl_isFullPathInCSL(const char *fullPath, const char *CSLBuf);


/** Generate a specially formatted difference listing of LowerLayer fullpaths
 *  for use in CMS_MSG_INTFSTACK_LOWERLAYER_UPDATE msg.
 */
CmsRet cmsUtl_diffFullPathCSLs(const char *newLowerLayerBuf,
                               const char *currLowerLayerBuf,
                               char *diffCSLBuf, UINT32 CSLlen);

#endif /* __CMS_STRCONV2_H__ */


