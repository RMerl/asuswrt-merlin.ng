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

#ifndef __CMS_PHL_H__
#define __CMS_PHL_H__

/*!\file cms_phl.h
 * \brief Public header file for the CMS Parameter Handler Layer (PHL) API.
 * Code which need to call PHL API functions must include this file.
 *
 * Here is a general description of how to use this interface.
 *
 */

#include "cms.h"
#include "cms_mdm.h"


/** Set Parameter Value structure
 */
typedef struct
{
   MdmPathDescriptor pathDesc;   /**< Full path name of a parameter. */
   char              *pParamType; /**< pointer to parameter type string */
   char              *pValue;    /**< Pointer to parameter value string. */
   CmsRet            status;     /**< Paremeter value set status. */
                                                                               
} PhlSetParamValue_t;

/** Get Parameter Value structure
 */
typedef struct
{
   MdmPathDescriptor pathDesc;   /**< Full path name of a parameter. */
   char              *pValue;    /**< Pointer to parameter value string. */
   const char        *pParamType;/**< Pointer to Parameter type string. */
                                                                               
} PhlGetParamValue_t;

/** Parameter Name-Property structure
 */
typedef struct
{
   MdmPathDescriptor pathDesc;   /**< Full path name of a parameter or an object. */
   const char       *profile;    /**< Parameter profile name */
   UBOOL8            writable;   /**< Writable property of the parameter or object.
                                  * Whether or not the Parameter value can be
                                  * overwritten using the SetParameterValues function.
                                  * If Name is a child object, this indicates whether
                                  * the DelObjInstance or AddObjInstance functions
                                  * can be used to remove this instance or add other
                                  * instances.
                                  */
   UBOOL8        isTr69Password; /**< Is this parameter considered a password in
                                  * the TR-069 protocol. */                       
} PhlGetParamInfo_t;


/** Parameter Attributes structure
 */
typedef struct
{
   MdmPathDescriptor pathDesc;   /**< Full path name of a parameter or an object. */
   MdmNodeAttributes attributes;	/**< Parameter attribute settings. */
} PhlGetParamAttr_t;


/** Set Parameter Attributes structure
 */
typedef struct
{
   MdmPathDescriptor pathDesc;   /**< Full path name of a parameter or an object. */
   MdmNodeAttributes attributes;	/**< Parameter attribute settings. */
} PhlSetParamAttr_t;


/** Set Parameter Values.
 * This API function may be used by a management entity to modify the value
 * of one or more CPE parameters.  Once called, this function will apply the
 * changes to each of the specified parameters atomically.  The application
 * of value changes to the CPE is independent from the order in which they
 * are listed. If the application of changes is successful, the new parameter
 * values will be set in the In-memory Data Model. Otherwise, all changes
 * will be discarded.  If the CPE requires a reboot before applying the
 * parameter values, this function will validate all change requests, set
 * the new parameter values in the In-memory Data Model, save the CPE
 * configuration to Flash, and return with the SUCCESS_REBOOT_REQUIRED status
 * code.
 *
 * @param pSetParamValueList (IN) Pointer to an array of PhlSetParamValue_t structs.
 *                           (OUT) On return, if there is a fault due to one
 *                           or more parameters in error, the fault code of
 *                           each parameter in error will be returned in its
 *                           status field. 
 * @param numEntries (IN) Number of entries in pSetParamValueList.
 * @return SUCCESS or SUCCESS_REBOOT_REQUIRED or one of the following fault codes:
 *         REQUEST_DENIED, INTERNAL_ERROR, INVALID_ARGUMENTS, RESOURCE_EXCEEDED,
 *         INVALID_PARAM_NAME, INVALID_PARAM_TYPE, INVALID_PARAM_VALUE,
 *         SET_NON_WRITABLE_PARAM.   
 *         If there is a fault due to one or more parameters in error, the fault
 *         code of each parameter in error will be returned in the status field
 *         of the parameter in the input pParamList.  In this case, this function
 *         will return fault code INVALID_ARGUMENTS.
 */
CmsRet cmsPhl_setParameterValues(PhlSetParamValue_t   *pSetParamValueList,
                                 SINT32               numEntries);

/** Get Parameter Values.
 * This API function may be used by a management entity to get the value of
 * one or more CPE parameters.  If the input name path is a parameter name,
 * this function will return the value of the parameter. If the input name
 * path is an object name and the nextLevelOnly boolean is FALSE, this
 * function will return the value of all the parameters containing in the
 * object and in the subtrees below it. If the input name path is an object
 * name and the nextLevelOnly boolean is TRUE, this function will return the
 * values of all parameters containing in the object only, not those in the
 * subtrees below it.
 *
 * @param pNamePathList (IN) Pointer to an array of MdmPathDescriptor structs.
 * @param numNameEntries (IN) Number of entries in pNamePathList.
 * @param nextLevelOnly (IN) FALSE- The return lists the value of all the
 *                           parameters containing in an object and in
 *                           the subtrees below it.
 *                           TRUE-  The return lists the value of all the
 *                           parameters containing in an object only. 
 * @param pParamValueList (OUT) Pointer to an array of ParamValue structs.
 * @param pNumParamValueEntries (OUT) Number of entries in pParamValueList
 * @return SUCCESS or one of the following fault codes:
 *         REQUEST_DENIED, INTERNAL_ERROR, INVALID_ARGUMENTS, RESOURCE_EXCEEDED,
 *         INVALID_PARAM_NAME.   
 */
CmsRet cmsPhl_getParameterValues(MdmPathDescriptor    *pNamePathList,
                                 SINT32               numNameEntries,
                                 UBOOL8               nextLevelOnly,
                                 PhlGetParamValue_t   **pParamValueList,
                                 SINT32               *pNumParamValueEntries);

/** Get Parameter Names.
 * This API function may be used by a management entity to discover the
 * writable property of parameters on a particular CPE.  If the input
 * name path is a parameter name, this function will return the writable
 * property of the parameter.  If the input name path is an object name
 * and the nextLevelOnly boolean is FALSE, this function will return the
 * writable properties of all the parameters and child objects containing
 * in the object and in the subtrees below it.  If the input name path is
 * an object name and the nextLevelOnly boolean is TRUE, this function will
 * return only the writable properties of all the parameters and child
 * objects containing in the object only, but not those in the subtrees
 * below it.
 *
 * @param pNamePath (IN) The full path name of a parameter or an object.
 * @param nextLevelOnly (IN) FALSE- the return lists the writable properities
 *                           of all the parameters and child objects containing
 *                           in an object and in the subtrees below it.
 *                           TRUE-  the return lists the writable properties
 *                           of all the parameters and child objects containing
 *                           in an object only. 
 * @param pParamInfoList (OUT) Pointer to an array of PhlGetParamInfo_t structs.
 * @param pNumEntries (OUT) Number of entries in pParamInfoList.
 * @return SUCCESS or one of the following fault codes:
 *         REQUEST_DENIED, INTERNAL_ERROR, INVALID_ARGUMENTS, INVALID_PARAM_NAME.   
 */
CmsRet cmsPhl_getParameterNames(MdmPathDescriptor *pNamePath,
                                UBOOL8            nextLevelOnly,
                                PhlGetParamInfo_t **pParamInfoList,
                                SINT32            *pNumEntries);

/** Set Parameter Attributes.
 * This API function may be used by the OWNER (management entity) of the
 * configuration data model to modify attributes associated with one or
 * more CPE parameters.  If the input name path is a parameter name, the
 * attribute of the parameter will be modified.  If the input name path
 * is an object name and the nextLevelOnly boolean is FALSE, the new
 * attributes will be applied to all the parameters containing in the
 * object and in the subtrees below it. If the input name path is an
 * object name and the nextLevelOnly boolean is TRUE, the new attributes
 * will be applied to all the parameters containing in the object only,
 * not those in the subtrees below it.
 * NOTE: Only the owner of the entire data model can modify parameter
 * attributes.  The owner of the data model is assigned at compile time
 * and has the highest priority in acquiring the write lock to the
 * data model.
 *
 * @param pSetParamAttrList (IN) Pointer to an array of PhlSetParamAttr_t structs.
 * @param numEntries (IN) Number of entries in pSetParamAttrList.
 * @return SUCCESS or one of the following fault codes:
 *         REQUEST_DENIED, INTERNAL_ERROR, INVALID_ARGUMENTS, RESOURCE_EXCEEDED,
 *         INVALID_PARAM_NAME, NOTIFICATION_REQ_REJECTED.   
 */
CmsRet cmsPhl_setParameterAttributes(PhlSetParamAttr_t *pSetParamAttrList,
                                     SINT32            numEntries);


/** Get Parameter Attributes.
 * This API function may be used by a management entity to read the attributes
 * associated with one or more CPE parameters.  If the input name path is a 
 * parameter name, this function will return the attributes of the specified
 * parameter. If the input name path is an object name and the nextLevelOnly
 * boolean is FALSE, attributes of all the parameters containing in the object
 * and in the subtrees below it will be returned. If the input name path is
 * an object name and the nextLevelOnly UBOOL8ean is TRUE, attributes of all the
 * parameters containing in the object only, not those in the subtrees below it,
 * will be returned.
 *
 * @param pNamePathList (IN) Pointer to an array of MdmPathDescriptor structs.
 * @param numNameEntries (IN) Number of entries in pNamePathList.
 * @param nextLevelOnly (IN) FALSE- Get attributes of all the parameters containing
 *                           in the object and in the subtrees below it
 *                           TRUE-  Get attributes of all the parameters containing
 *                           in the object only.
 * @param pParamAttrList (OUT) Pointer to an array of PhlGetParamAttr_t structs.
 *                             Caller is responsible for freeing this buffer
 *                             by simply calling cmsMem_free on it.
 * @param pNumParamAttrEntries (OUT) Number of entries in pParamAttrList.
 * @return SUCCESS or one of the following fault codes:
 *         REQUEST_DENIED, INTERNAL_ERROR, INVALID_ARGUMENTS, RESOURCE_EXCEEDED,
 *         INVALID_PARAM_NAME.   
 */
CmsRet cmsPhl_getParameterAttributes(MdmPathDescriptor *pNamePathList,
                                     SINT32            numNameEntries,
                                     UBOOL8            nextLevelOnly,
                                     PhlGetParamAttr_t **pParamAttrList,
                                     SINT32            *pNumParamAttrEntries);

/** Add Object Instance.
 * This API function may be used by a management entity to create a new
 * instance of a multi-instance object.  The function call takes as an
 * argument the path name of the collection of objects for which a new
 * instance is to be created. For example:
 *    top.group.object.
 * This path name does not include an instance number for the object to
 * be created.  That instance number is assigned by the CMS and returned
 * to the caller.  Once assigned the instance number of an object cannot
 * be changed and persists until the object is deleted using the
 * delObjInstance function.  After creation, parameters or child objects
 * within the object are referenced by the path name appended with the
 * instance ID.  For example, if the addObjInstance function returned
 * an instance ID of 2, a parameter within this instance may then
 * be referred to by the path:
 *    top.group.object.2.parameter
 * On creation of an object instance using this function, the parameters
 * contained within the object and in the subtrees below it are set to
 * their default values and the associated attributes are set to the
 * following:
 *    - Notification is set to zero (notification off)
 *    - AccessList includes all defined entities
 *
 * @param pNamePath (IN/OUT) On input, the full path name of an object
 *                           for which a new instance is to be created.
 *                           On output, the full path name of the new
 *                           object instance. The new instance ID can
 *                           be obtained from the top of the instance
 *                           ID stack.
 * @return SUCCESS or one of the following fault codes:
 *         REQUEST_DENIED, INTERNAL_ERROR, INVALID_ARGUMENTS, RESOURCE_EXCEEDED,
 *         INVALID_PARAM_NAME.   
 */
CmsRet cmsPhl_addObjInstance(MdmPathDescriptor *pNamePath);

/** Delete Object Instance.
 * This API function may be used by a management entity to delete a
 * a particular instance of a multi-instance object.  The function call
 * takes as an argument the full path name of the object instance
 * including the instance ID. For example:
 *    top.group.object.2.
 * When this function call is successful, the object instance and the
 * subtrees below it will be deleted.
 * When an object instance is deleted, the instance IDs associated
 * with any other instances of the same collection of objects remain
 * unchanged.  Thus, the instance IDs of object instances in a
 * collection might not be consecutive.
 *
 * @param pNamePath (IN) The full path name of an object instance.
 * @return SUCCESS or one of the following fault codes:
 *         REQUEST_DENIED, INTERNAL_ERROR, INVALID_ARGUMENTS, INVALID_PARAM_NAME.   
 */
CmsRet cmsPhl_delObjInstance(MdmPathDescriptor *pNamePath);


CmsRet cmsPhl_getNextPath(UBOOL8             paramOnly,
                          UBOOL8             nextLevelOnly,
                          MdmPathDescriptor  *pRootPath,
                          MdmPathDescriptor  *pPath);

CmsRet cmsPhl_getPathCount(MdmPathDescriptor *pPathList,
                           SINT32            numEntries,
                           UBOOL8            paramOnly,
                           UBOOL8            nextLevelOnly,
                           SINT32            *pPathCnt);

CmsRet cmsPhl_getParamInfo(MdmPathDescriptor *pPath,
                           PhlGetParamInfo_t **pParamInfo);

CmsRet cmsPhl_getParamValue(MdmPathDescriptor  *pPath,
                            PhlGetParamValue_t **pParamValue);

CmsRet cmsPhl_getParamAttr(MdmPathDescriptor *pPath,
                           PhlGetParamAttr_t **pParamAttr);

/** Free Get Parameter Value Buffer.
 * This API function may be used to free the memory allocated
 * for the PhlGetParamValue_t buffer.
 *
 * @param pBuf (IN) pointer to the buffer.
 * @param numEntries (IN) number of PhlGetParamValue_t entries in the buffer.
 * @return void
 */
void cmsPhl_freeGetParamValueBuf(PhlGetParamValue_t *pBuf,
                                 SINT32             numEntries);


/** Check if parameter value has changed.
 * 
 * Note that this function should only be called on parameters
 * which has active or passive notification attribute set.
 * Calling this function with a parameter which does not have
 * active or passive notification attribute will always result in
 * FALSE return value, even though the value may have been changed.
 *
 * @param *pathDesc (IN) Pointer to the parameter name path descriptor.
 * @return TRUE if the parameter value has been changed.  FALSE otherwise.
 */
UBOOL8 cmsPhl_isParamValueChanged(const MdmPathDescriptor *pathDesc);


/** Get number of parameters whose value has changed and who has
 *  either passive or active change notification attribute.
 *
 * @return The total number of parameter value changes.
 */
UINT32 cmsPhl_getNumberOfParamValueChanges(void);


/** Clear Parameter Value Change status for all values in the MDM.
 *
 * This function clears the status of all the parameter value changes.
 */
void cmsPhl_clearAllParamValueChanges(void);


#endif /* __CMS_PHL_H__ */
