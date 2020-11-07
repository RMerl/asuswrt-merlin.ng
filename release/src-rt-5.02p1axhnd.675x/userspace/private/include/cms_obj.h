/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
 ************************************************************************/

#ifndef __CMS_OBJ_H__
#define __CMS_OBJ_H__


/*!\file cms_obj.h
 * \brief Header file for the CMS Object Layer API.
 *  This is in the cms_core library.
 */

#include "cms.h"
#include "cms_mdm.h"
#include "mdm_objectid.h"
#include "mdm_object.h"


/** Get the current values in the MDM; call STL handler function to update values. This is used to get values 
 * in normal everyday scenarios
 *
 * This flag can be used with cmsObj_get and all of the cmsObj_getNext and cmsObj_getAncestor().
 */
#define OGF_NORMAL_UPDATE    0x0000

/** Get default values of the object instead of the current values.
 *
 * This flag is used in the getFlags param of cmsObj_get only.
 */
#define OGF_DEFAULT_VALUES    0x0001

/** Get the current values in the MDM; do not call STL handler function to update values.
 *
 * This flag can be used with cmsObj_get and all of the cmsObj_getNext and cmsObj_getAncestor().
 */
#define OGF_NO_VALUE_UPDATE   0x0002



/** This structure is used by the cmsObj_getNthParam() API to return information
 * about the Nth parameter of a MdmObject.
 *
 * @param totalParams - Total number of parameters stored in a MdmObject.
 * @param name - Parameter name.
 * @param type - Parameter type, as defined in MdmParamTypes enum.
 * @param minVal - Parameter's minimum value limit. For the types MPT_STRING,
 *                 MPT_DATE_TIME, and MPT_BASE64, minVal has no meaning.
 * @param maxVal - Parameter's maximum value limit.
 *                 For the MPT_STRING type, maxVal contains the maximum number
 *                 of characters that can be stored in the character string
 *                 (excluding the terminating NULL character).
 *                 For the MPT_DATE_TIME type, maxVal has no meaning.
 *                 For the MPT_BASE64 type, maxVal contains the maximum number
 *                 of bytes that can be stored in the binary string.
 * @param val - Pointer to the parameter value.
 */
typedef struct
{
    UINT32 totalParams;
    char name[MAX_MDM_PARAM_NAME_LENGTH + 1];
    MdmParamTypes type;
    UINT32 minVal;
    UINT32 maxVal;
    void *val;

} MdmObjParamInfo;



#ifdef __cplusplus
extern "C" {
#endif

/** Get a MdmObject.
 *
 * @param oid (IN)  The oid of the MdmObject to get.
 * @param iidStack (IN) The instance information for the MdmObject to get.
 *                      If OGF_DEFAULT_VALUES is set, this parameter is ignored.
 * @param getFlags (IN) Additional flags for the get, see OGF_xxx. If getFlags is
 *                      0, then the current values of the mdmObj is returned.
 * @param obj (OUT) On successful return, obj points to the requested MdmObject.
 *                  The caller must free this object by calling cmsObj_free()
 *                  (not cmsMem_free).
 * @return CmsRet enum.
 */
CmsRet cmsObj_get(MdmObjectId oid,
                  const InstanceIdStack *iidStack,
                  UINT32 getFlags,
                  void **mdmObj);




/** Get the next instance of the object.
 *
 * This function can only be used for objects that can have multiple
 * instances.  If an object can only have one instance, use cmsObj_get().
 * This function just calls cmsObj_getNextInSubTree with parentIidStack = NULL.
 *
 * @param oid (IN) The oid of the MdmObject to get.
 * @param iidStack (IN/OUT) The instance information for the current instance
 *                      of the object.  If the iidStack is freshly
 *                      initialized (empty), then the first instance
 *                      of the object will be returned.  Otherwise,
 *                      the next instance from the instance indicated
 *                      by iidStack will be returned.
 * @param obj (OUT) On successful return, obj points to the next
 *                  MdmObject.  The caller must free this object by
 *                  calling cmsObj_free (not CmsMem_free).
 * @return CmsRet enum.
 */
CmsRet cmsObj_getNext(MdmObjectId oid,
                      InstanceIdStack *iidStack,
                      void **mdmObj);



/** Same as cmsObj_getNext() but this one accepts a flags argument.
 */
CmsRet cmsObj_getNextFlags(MdmObjectId oid,
                           InstanceIdStack *iidStack,
                           UINT32 getFlags,
                           void **mdmObj);



/** Get the next instance of the object under the specified parent instance.
 *
 * This function can only be used for objects that can have multiple
 * instances.  If an object can only have one instance, use cmsObj_get().
 *
 * @param oid (IN) The oid of the MdmObject to get.
 * @param parentIidStack (IN) If the iidStack of the parent sub-tree.
 *                            This parentIidStack constrains the search
 *                            of the getNextInSubTree function to only return
 *                            mdmObjects within a certain sub-tree.
 *                            This param may be null if no sub-tree checking is needed.
 * @param iidStack (IN/OUT) The instance information for the current instance
 *                      of the object.  If the iidStack is freshly
 *                      initialized (empty), then the first instance
 *                      of the object will be returned.  Otherwise,
 *                      the next instance from the instance indicated
 *                      by iidStack will be returned.
 * @param obj (OUT) On successful return, obj points to the next
 *                  MdmObject.  The caller must free this object by
 *                  calling cmsObj_free (not CmsMem_free).
 * @return CmsRet enum.
 */
CmsRet cmsObj_getNextInSubTree(MdmObjectId oid,
                               const InstanceIdStack *parentIidStack,
                               InstanceIdStack *iidStack,
                               void **mdmObj);



/** Same as cmsObj_getNextInSubTree() but this one accepts a flags argument.
 */
CmsRet cmsObj_getNextInSubTreeFlags(MdmObjectId oid,
                                    const InstanceIdStack *parentIidStack,
                                    InstanceIdStack *iidStack,
                                    UINT32 getFlags,
                                    void **mdmObj);



/** Get a copy of the MdmObject that is the ancestor (higher in the sub-tree) of
 * the specified decendent object.
 *
 * Specifically, ancestor can be parent, grand parent, great-grand-parent,
 * or uncle, great-uncle, or great-great-uncle of the decendent.
 * But ancestor does not include the brothers of the decendent.
 * From the data model point of view, the ancestor object can be
 * uniquely identified with the information in the iidStack of the
 * decendent and the ancestor can claim that the decendent is in its sub-tree.
 *
 * @param ancestorOid (IN) The oid of the desired ancestor object.
 * @param decendentOid (IN) The oid of the decendent object.
 * @param iidStack (IN/OUT) On entry, this will be the iidStack of the
 *                          decendent object.  On successful return,
 *                          this will contain the iidStack of the 
 *                          ancestor object.
 * @param mdmObj (OUT) If successful, a copy of the requested MdmObject will
 *                     be returned.  If caller makes any modifications to the
 *                     object, caller must call mdm_setObject for the changes
 *                     to be stored in the MDM.  Caller is responsible for
 *                     freeing the MdmObject.
 * @return CmsRet enum.
 */
CmsRet cmsObj_getAncestor(MdmObjectId ancestorOid,
                          MdmObjectId decendentOid,
                          InstanceIdStack *iidStack,
                          void **mdmObj);



/** Same as cmsObj_getAncestor() but this one accepts a flags argument.
 */
CmsRet cmsObj_getAncestorFlags(MdmObjectId ancestorOid,
                               MdmObjectId decendentOid,
                               InstanceIdStack *iidStack,
                               UINT32 getFlags,
                               void **mdmObj);



/** Get information about the Nth parameter of a MdmObject.
 *
 * @param mdmObj (IN) Pointer to the MdmObject from which the parameter
 *                    information will be retrieved.
 * @param paramNbr (IN) Parameter number, where 0 corresponds to the first
 *                      parameter of the MdmObject.
 * @param paramInfo (OUT) Pointer to a MdmObjParamInfo structure used to return
 *                        information about the the parameter number
 *                        'paramNbr' of the MdmObject.
 *                        The user of this API is responsible for allocating and
 *                        de-allocating the memory used to store the
 *                        MdmObjParamInfo structure.
 * @return CmsRet enum.
 */
CmsRet cmsObj_getNthParam(const void *mdmObj,
                          const UINT32 paramNbr,
                          MdmObjParamInfo *paramInfo);




/** Free a MdmObject returned by cmsObj_get and cmsObj_getnext.
 *
 * The function knows which object is given to it by the embedded
 * oid field in the object structure.
 *
 * @param mdmObj (IN) Address of pointer to the MdmObject to be freed.
 */
void cmsObj_free(void **mdmObj);




/** Set (write) the object.
 *
 * Caller is responsible for freeing the MdmObject regardless
 * of the return value from this function.
 *
 * @param mdmObj   (IN) Pointer to the modified MdmObject.
 * @param iidStack (IN) The instance information of the MdmObject
 *                      to be written to.
 *
 * @return CmsRet enum.
 */
CmsRet cmsObj_set(const void *mdmObj,
                  const InstanceIdStack *iidStack);


/** Just do the cmsObj_set normally (this flag has no bits set).
 */
#define OSF_NORMAL_SET        0x0000


/** Just set the object into the MDM, but do not call the RCL handler function.
 */
#define OSF_NO_RCL_CALLBACK    0x0001


/** Do not check read/write or access permission before the set.
 *
 * Obviously, this is a dangerous option since it allows the caller to
 * bypass the normal checking.  So use this flag carefully and wisely.
 * Currently, it is used by tr69c to write to the read-only ParameterKey
 * field in the ManagementServer object.
 */
#define OSF_NO_ACCESSPERM_CHECK    0x0002


/** Set (write) the object with option flags.
 *
 * Caller is responsible for freeing the MdmObject regardless
 * of the return value from this function.
 *
 * @param mdmObj   (IN) Pointer to the modified MdmObject.
 * @param iidStack (IN) The instance information of the MdmObject
 *                      to be written to.
 * @param flags    (IN) One or more of the OSF_xxx defines above.
 *
 * @return CmsRet enum.
 */
CmsRet cmsObj_setFlags(const void *mdmObj,
                       const InstanceIdStack *iidStack,
                       UINT32 flags);


/** Set the value of the Nth parameter of a MdmObject.
 *
 * @param mdmObj (IN) Pointer to the MdmObject that holds the parameter that
 *                    will be updated.
 * @param paramNbr (IN) Parameter number, where 0 corresponds to the first
 *                      parameter of the MdmObject.
 * @param paramInfo (OUT) Pointer to the memory location that contains the new
 *                        value to be stored in the parameter number 'paramNbr'
 *                        of the MdmObject.
 * @return CmsRet enum.
 */
CmsRet cmsObj_setNthParam(void *mdmObj,
                          const UINT32 paramNbr,
                          const void *paramVal);




/** Clear (zeroize) the statistics in the specified object.
 *
 * Note that you can only clear all of the statistics in an
 * object.  It is not possible to clear just one statistics field
 * in an object.
 *
 * @param oid (IN) The oid of the MdmOjbect whose statistics are
 *                 to be cleared.
 * @param iidStack (IN) The instance information of the MdmObject
 *                      whose statistics are to be cleared.
 * @return CmsRet enum.
 */
CmsRet cmsObj_clearStatistics(MdmObjectId oid,
                              const InstanceIdStack *iidStack);


/** Create an instance of an object.
 *
 * @param oid (IN) The oid of the object node above the instance node where
 *                 the new instance is to be created.  For example, if you want
 *                 to create another instance of "InternetGatewayDevice.
 *                 WANDevice.1.ConnectionDevice.x", the oid should be the
 *                 one that refers to "InternetGatewayDevice.WANDevice.1.
 *                 ConnectionDevice".  "x" will be the newly created instance
 *                 number.
 * @param iidStack (IN/OUT)  On entry, the iidStack contains instance information
 *                 up to the oid in the MDM tree.  On successful return,
 *                 iidStack will contain one more instance element which is
 *                 the instance number that was created.
 * @return CmsRet enum.
 */
CmsRet cmsObj_addInstance(MdmObjectId oid,
                          InstanceIdStack *iidStack);


/** Delete an instance of an object.
 *
 * @param oid (IN) The oid of the object node above the instance node
 *                 where the instance is to be deleted.  This is the
 *                 same oid that was used in the cmsObj_createInstance.
 * @param iidStack (IN) The instance information for the instance to
 *                 be deleted.  Note that this variable is not modified
 *                 by the function even though on successful return,
 *                 the iidStack is not really valid because the last
 *                 instance on the stack no longer exists.
 * @return CmsRet enum.
 */
CmsRet cmsObj_deleteInstance(MdmObjectId oid,
                             const InstanceIdStack *iidStack);



/** Dumps all the parameters of a MdmObject.
 *
 * @param mdmObj (IN) Pointer to the MdmObject to be dumped.
 * @return CmsRet enum.
 */
CmsRet cmsObj_dumpObject(const void *mdmObj);

#define MAX_VALUE_LENGTH 256

typedef struct paramNodeList
{
   int offset;
   char value[MAX_VALUE_LENGTH]; 
   struct paramNodeList *nextNode;
} paramNodeList;

/** Compare all the parameters of two MdmObjects with the same OID.
 *
 * @param mdmObj1 (IN) Pointer to the first MdmObject to be compared.
 * @param mdmObj2 (IN) Pointer to the second MdmObject to be compared.
 * @param differedParamList (OUT) Pointer to the Paramemters with different value.
 * @return CmsRet enum.
 */
CmsRet cmsObj_compareObjects(const void *mdmObj1, const void *mdmObj2, paramNodeList **differedParamList);

/* Sets a particular instance of an object as Non-persistent i.e
 * the instance is not written to config-file/flash its just maintained
 * in MDM,so the instance will not sustain reboots.
 *
 * Note:cmsMgm_saveConfigToFlash has to called to remove any existing copy
 * from config file 
 *
 * @param oid (IN) The oid of the object node above the instance node
 *                 where the instance is created.
 * @param iidStack (IN) The instance information for the instance 
 */  
CmsRet cmsObj_setNonpersistentInstance(const MdmObjectId oid,
                             const InstanceIdStack *iidStack);

/* Makes a particular instance of an object as persistent. 
 * By default all the instances are persistent, this is can be of use
 * only if instance is set to non-persistent.
 *
 * Note:cmsMgm_saveConfigToFlash has to called to write a copy
 * to config file 
 *
 * @param oid (IN) The oid of the object node above the instance node
 *                 where the instance is created.
 * @param iidStack (IN) The instance information for the instance 
 */  
CmsRet cmsObj_clearNonpersistentInstance(const MdmObjectId oid,
                             const InstanceIdStack *iidStack);


/* Checks if a particular instance of an object is persistent or not. 
 *
 * @param oid (IN) The oid of the object node above the instance node
 *                 where the instance is created.
 * @param iidStack (IN) The instance information for the instance 
 */  
UBOOL8 cmsObj_isNonpersistentInstance(const MdmObjectId oid,
                             const InstanceIdStack *iidStack);


#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* __CMS_OBJ_H__ */
