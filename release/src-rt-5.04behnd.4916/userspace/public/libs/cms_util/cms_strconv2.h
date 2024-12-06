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


/** Return TRUE if fullpath contains an instance id range specifier.
 *  e.g. Device.QoS.Queue.[800000-899999].
 */
UBOOL8 cmsUtl_hasNamespaceRange(const char *fullpath);

/** Given a namespace with an instance id range specifier, 
 *  e.g. Device.QoS.Queue.[800000-899999].
 *  parse it and return results.
 *  @param (IN)  fullpath: the original fullpath
 *  @param (OUT) baseFullpath: this must point to a buffer big enough to hold the
 *         original fullpath.
 *  Return:
 *  On success, the firstInanceId, lastInstanceId, and baseFullpath will be filled in.
 *  If no range specifier, CMSRET_NO_MORE_INSTANCES
 *  Any other parsing or arg error, CMSRET_INVALID_ARGUMENTS.
 */
CmsRet cmsUtl_parseNamespaceRange(const char *fullpath,
      UINT32 *firstInstanceId, UINT32 *lastInstanceId, char *baseFullpath);


/** Given an input string, a start index and an end index, 
 *  this function returns a new string created using the substring sliced from the input.
 *  For example, for input string "This is a nice day", start=5, end=7, this function
 *  returns a new string "is".
 *  @param (IN)  input: the original input string. This is unchanged in the function.
 *  @param (IN)  start: the start index of the substring.
 *  @param (IN)  end:   the end index of the substring. Please note the end index points to 
 *  1 char ahead of the substring content. See the example above.
 *  Return:
 *  On success, a new string will be returned which will be the substring. The caller is
 *  responsible to call cmsMem_free on the returned substring.
 *  Otherwise NULL is returned.
 */
char *cmsUtl_substring(const char *input, size_t start, size_t end);

/** Given a MDM fullpath,
 *  e.g. Device.QoS.Queue.800000.
 *  This function parse it and return generic string and instanceStack, with only string
 *  manipulation.
 *  @param (IN)  fullpath: the original fullpath
 *  @param (OUT) pGenericPath: A pointer to hold the address of the returned genericPath string.
 *  @param (OUT) iidStack: A pointer to hold the returned iidStack.
 *  Return:
 *  On success, the pGenericPath, iidStack will be filled in, and CMSRET_SUCCESS is returned.
 *  Otherwise, the return code indicates the error.
 *  The caller is responsible to call cmsMem_free on the returned genericPath.
 */
CmsRet cmsUtl_parseFullpathToGeneric(const char *fullpath, char **pGenericPath,
                                     InstanceIdStack *iidStack);

/** Given a MDM generic path,
 *  e.g. Device.WiFi.Radio.{i}.Stats.{i}.
 *  and an iidStack, 
 *  This function parse and return a fullPath string by string manipulation.
 *  If the iidStack contains less number of instance depth than the genericPath,
 *  the returned fullPath will only show until the given depth of the iidStack.
 *  For example:
 *
 *  genericPath = "Device.WiFi.
 *  iidStack->currentDepth = 0
 *  The function returns:
 *  "Device.WiFi."
 *
 *  genericPath = "Device.WiFi.Radio.{i}.Stats.
 *  iidStack->currentDepth = 1, iidStack->instances[0] = 2
 *  The function returns:
 *  "Device.WiFi.Radio.2.Stats."
 *
 *  genericPath = "Device.WiFi.AccessPoint.{i}.AssociatedClients.{i}.
 *  iidStack->currentDepth = 2, iidStack->instances[0] = 2, iidStack->instances[1] = 1,
 *  The function returns:
 *  "Device.WiFi.AccessPoint.2.AssociatedClients.1."
 *
 *  genericPath = "Device.WiFi.AccessPoint.{i}.AssociatedClients.{i}.
 *  iidStack->currentDepth = 1, iidStack->instances[0] = 2
 *  The function returns:
 *  "Device.WiFi.AccessPoint.2"
 *
 *  @param (IN)  genericPath: the original genericPath
 *  @param (IN)  iidStack: the iidStack to be used
 *  @param (OUT) pFullPath: A pointer to hold the address of the returned fullpath string.
 *  @param (IN)  truncate: When set, if the given iidStack doesn't contain all needed depthes, trunca *               the the result. If not set, return error in this case.
 *  Return:
 *  On success, the pFullPath, will be filled in, and CMSRET_SUCCESS is returned.
 *  Otherwise, the return code indicates the error.
 *  The caller is responsible to call cmsMem_free on the returned fullpath string.
 */
CmsRet cmsUtl_genericPathToFullPath(const char *genericPath,
                                    const InstanceIdStack *iidStack,
                                    char **pFullPath,
                                    int truncate);
/*
 * The next 2 functions extract an alias from a fullpath.
 * For now, the alias must be at the end of the fullpath,
 * e.g. Device.QoS.Queue.[DSL-high-prio-queue].
 * In the future, these functions could be made smarter to extract aliases
 * at different locations in the fullpath, but that is not needed right now.
 */
UBOOL8 cmsUtl_hasInstanceAlias(const char *fullpath);

/** extract (end) alias from fullpath.
 *  @param (IN)  fullpath: the original fullpath
 *  @param (OUT) baseFullpath: this must point to a buffer big enough to hold the
 *         original fullpath.
 *  @param (OUT) aliasBuf: holds the extracted alias, if there is one.
 *  @param (IN) aliasBufLen: size of the aliasBuf.
 */
CmsRet cmsUtl_parseInstanceAlias(const char *fullpath,
                                 char *baseFullpath,
                                 char *aliasBuf, UINT32 aliasBufLen);


#endif /* __CMS_STRCONV2_H__ */


