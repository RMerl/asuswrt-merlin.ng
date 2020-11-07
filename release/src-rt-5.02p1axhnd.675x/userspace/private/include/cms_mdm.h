/*
* <:copyright-BRCM:2011:proprietary:standard
* 
*    Copyright (c) 2011 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
:>
*/

#ifndef __CMS_MDM_H__
#define __CMS_MDM_H__

/*!\file cms_mdm.h
 * \brief Header file containing MDM functions that external
 *  management entities are allowed to call.
 *
 */

#include "cms.h"
#include "cms_eid.h"
#include "mdm_params.h"


/** All MdmObjects start with these two fields (for internal use only) */
typedef struct _mdmObjectHeader {
    MdmObjectId _oid;
    UINT16      _sequenceNum;
} _MdmObjectHeader;

/** Get the MdmObjectId of the MDM object. */
#define GET_MDM_OBJECT_ID(obj)  (((_MdmObjectHeader *) (obj))->_oid)

/** Get the sequence number of the MDM object. */
#define GET_MDM_OBJ_SEQ_NUM(obj)  (((_MdmObjectHeader *) (obj))->_sequenceNum)


/** Macro to zeroize/initialize an InstanceIdStack
 *
 */
#define INIT_INSTANCE_ID_STACK(s)  (memset((void *)(s), 0, sizeof(InstanceIdStack)))


/** definition of empty InstanceIdStack; this can be used to set an
 * InstanceIdStack variable declared on the stack to the empty state.
 * Hmm, if the user adds another level to instance depth, will this
 * define have to be changed?  Or will the compiler automatically extend
 * the {0,0,0,0} to {0,0,0,0,0}?
 */
#define EMPTY_INSTANCE_ID_STACK {0, {0,0,0,0}}

/** Macro to push another instance id onto the instance stack. */
#define PUSH_INSTANCE_ID(s, id) \
   do {if ((s)->currentDepth < MAX_MDM_INSTANCE_DEPTH ) { \
         (s)->instance[(s)->currentDepth] = (id);    \
         (s)->currentDepth++; }                       \
   } while(0)


/** Macro to pop the topmost instance id from the stack. */
#define POP_INSTANCE_ID(s) \
   ((s)->currentDepth > 0) ? (s)->instance[(((s)->currentDepth)--) - 1] : 0


/** Return the topmost instance id from the stack, but does not modify stack. */
#define PEEK_INSTANCE_ID(s) \
   ((s)->currentDepth > 0) ? (s)->instance[(s)->currentDepth - 1] : 0


/** Return the instance id from the specified depth */
#define INSTANCE_ID_AT_DEPTH(s, depth) \
   (((depth) < (s)->currentDepth) ? (s)->instance[(depth)] : 0)

/** Macro to get the depth of the instance id stack */
#define DEPTH_OF_IIDSTACK(s) ((s)->currentDepth)


/** Instance number for ATM WANDevice */
#define CMS_WANDEVICE_ATM        1

/** Instance number for PTM WANDevice */
#define CMS_WANDEVICE_PTM        2

/** Instance number for Ethernet WANDevice */
#define CMS_WANDEVICE_ETHERNET   3

/** Instance number for Moca WANDevice */
#define CMS_WANDEVICE_MOCA       4

/** Instance number for Gpon WANDevice */
#define CMS_WANDEVICE_GPON       5

/** Instance number for Epon WANDevice */
#define CMS_WANDEVICE_EPON       6

/** Instance number for Wl WANDevice */
#define CMS_WANDEVICE_WIFI       7

/** Instance number for Bonded ATM WanDevice */
#define CMS_WANDEVICE_ATMBONDING 12

/** Instance number for Bonded PTM WanDevice */
#define CMS_WANDEVICE_PTMBONDING 13

/** Starting Instance number for L2TP Access Concentrator (LAC) WANDevice */
#define CMS_WANDEVICE_L2TPAC     100

/** (maximum) Number of L2TP AC tunnels supported */
#define CMS_WANDEVICE_L2TPAC_COUNT 1

/** Starting Instance number for PPTP Access Concentrator (LAC) WANDevice */
#define CMS_WANDEVICE_PPTPAC     101

/** (maximum) Number of PPTP AC tunnels supported */
#define CMS_WANDEVICE_PPTPAC_COUNT 1



/*!\enum MdmParamTypes
 * \brief Possible types for parameters in the MDM.
 *
 * The first 6 (MPT_STRING through MPT_BASE64) are from the TR-098 spec.
 * The next 3 (MPT_HEX_BINARY through MPT_UNSIGNED_LONG64) were introduced in
 * TR-106 Issue 1, Admendent 2, Sept 2008.
 * Broadcom does not define any additional types for now.
 */
/** UPDATE for TR-106 Issue1. : referece to https://www.broadband-forum.org/cwmp/tr-106-1-1-0-types.html
 *  support non-primitive data type
 *  UUID/IPAddress/MACAddress/StatsCounter32/StatsCounter64
 */

#define  MPT_BASE_TYPE_FILTER  0x00FF /** the filter primitive data types */

typedef enum
{
   MPT_STRING =           0x0000, /**< string. */
   MPT_INTEGER =          0x0001, /**< Signed 32 bit integer. */
   MPT_UNSIGNED_INTEGER = 0x0002, /**<Unsigned 32 bit integer. */
   MPT_BOOLEAN =          0x0003, /**< Either 1 (true) or 0 (false). */
   MPT_DATE_TIME =        0x0004, /**< string, in UTC, in ISO 8601 date-format. */
   MPT_BASE64 =           0x0005, /**< Base64 string representation of binary data. */
   MPT_HEX_BINARY =       0x0006, /**<Hex string representation of binary data. */
   MPT_LONG64 =           0x0007, /**< signed 64 bit integer */
   MPT_UNSIGNED_LONG64 =  0x0008, /**< unsigned 64 bit integer */
   /** The following date type are introduced by TR106. */
   MPT_UUID =             0x0100, /**< base type: string, Universally Unique Identifier */
   MPT_IP_ADDR =          0x0200, /**< base type: string, IP address, i.e. IPv4 address (or IPv4 subnet mask) or IPv6 address.*/
   MPT_MAC_ADDR =         0x0300, /**< base type: string, All MAC addresses are represented as strings of 12 hexadecimal digits.*/
   MPT_STATS_COUNTER32 =  0X0402, /**< base type: unsignedInt, A 32-bit statistics parameter, e.g. a byte counter. */
   MPT_STATS_COUNTER64 =  0X0508, /**< base type: unsignedLong, A 64-bit statistics parameter, e.g. a byte counter. */
} MdmParamTypes;


/** Return string representation for the given MdmParamType
 *
 * @param paramType (IN) MdmParamType
 * @return string representation.  This is a const string pointer.  The caller
 *         must not modify or free this pointer.
 */
const char *cmsMdm_paramTypeToString(MdmParamTypes paramType);




/** Active notification for TR69;  Used in MdmNodeAttributes.notification. */
#define NDA_TR69_NO_NOTIFICATION      0

/** Passive notification for TR69;  Used in MdmNodeAttributes.notification. */
#define NDA_TR69_PASSIVE_NOTIFICATION 1

/** Active notification for TR69;  Used in MdmNodeAttributes.notification. */
#define NDA_TR69_ACTIVE_NOTIFICATION  2



/** A structure for storing node attributes.
 *
 * The MdmNodeAttributes only has meaning when applied to a
 * parameter node.  However, to save memory, if all 
 * parameter nodes of an object instance have the same attributes, 
 * their attributes are stored in the next higher level structure,
 * which may be the MdmObjNode (indirect 0), the InstanceHeadNode (indirect 1),
 * or InstanceDescNode (indirect 2).
 *
 * The accessBitMask has 15 bits as defined by the NDA_ACCESS_xxx defines
 * in cms_eid.h.  The bitmap represents entities for which write access to
 * the specified parameter(s) is granted by the owner of the entire data model.
 *
 * The parameter notification attribute indicates
 * whether the CPE should include changed values
 * of the specified parameter in the inform message,
 * and whether the CPE must initiate a session to
 * the TR-069 ACS when the specified parameter(s)
 * change in value. The following values are defined:
 *
 * 0 = Notification off.  The CPE need not inform 
 *       the ACS of a change to the specified
 *       parameter(s).
 *
 * 1 = Passive notification. Whenever the specified
 *       parameter value changes, TR-069C must include
 *       the new value in the ParameterList in the
 *       Inform message that is sent the next time a
 *       session is established to the ACS.
 *
 * 2 = Active notification. Whenever the specified
 *       parameter value changes, TR-069C must
 *       initiate a session to the ACS, and include
 *       the new value in the ParameterList in the
 *       associated Inform message.
 */
typedef struct
{
   UINT16  accessBitMaskChange:1; /**< Used during set, indicates whether the accessBitMask should be set */
   UINT16  accessBitMask:15;      /**< Bit field containing access bit mask, see NDA_ACCESS_xxx. */
   UINT8   notificationChange:1;  /**< Used during set, indicates whether the notifiction field should be set */
   UINT8   notification:7;        /**< Type of notification requested. */
   UINT8   valueChanged:1;        /**< This param has active or passive notification, and its value has changed.
                                   *   This field is used by MDM internal storage.  Not used during set. */
   UINT8   reserved:7;            /**< Reserved bit fields. */
   UINT16  ownerId;               /**< Deprecated. Do not use. */ 
} MdmNodeAttributes;



/** Default notification value is no notification. */
#define DEFAULT_NOTIFICATION_VALUE   NDA_TR69_NO_NOTIFICATION

/** Default access list bitmask is write access granted to everybody. */
#define DEFAULT_ACCESS_BIT_MASK   (NDA_ACCESS_TR69C | NDA_ACCESS_SUBSCRIBER)

/** definition of emtpy MdmNodeAttributes, this can be used to set an
 * MdmNodeAttributes variable declared on the stack to the empty state.
 */
#define EMPTY_NODE_ATTRIBUTES {0,0,0,0,0,0,0}


/** Macro to zeroize/initialize a MdmNodeAttributes */
#define INIT_NODE_ATTRIBUTES(s)  (memset((void *)(s), 0, sizeof(MdmNodeAttributes)))




/** A compact representation of a fullpath that is easier for the MDM to handle.
 *
 * This structure can be used to represent a specific instance of an
 * object or parameter.
 */
typedef struct
{
   MdmObjectId       oid;        /**< Object Identifier.
                                  * Just a number used to represent a generic
                                  * object.
                                  */
   InstanceIdStack   iidStack;   /**< Instance Id Stack.
                                  * A stack of instance IDs used together with
                                  * the oid to specify a specific instance of
                                  * an object.
                                  */
   char paramName[MAX_MDM_PARAM_NAME_LENGTH + 1]; /**< Parameter name.
                                  * If paramName is an empty string, then this
                                  * MdmPathDescriptor structure specifies an object.
                                  * Otherwise, this structure specifies a
                                  * full path to a specific parameter.
                                  */
} MdmPathDescriptor;


/** Test whether there is a paramName in MdmPathDescriptor */
#define IS_PARAM_NAME_PRESENT(p) ((p)->paramName[0] != 0)

/** Macro to zeroize/initialize the paramname field of MdmPathDescriptor */
#define INIT_MDM_PARAM_NAME(p) (memset((void *)(p), 0, MAX_MDM_PARAM_NAME_LENGTH+1))


/** Macro to zeroize/initialize a MdmPathDescriptor */
#define INIT_PATH_DESCRIPTOR(p) (memset((void *)(p), 0, sizeof(MdmPathDescriptor)))


/** definition of empty MdmPathDescriptor; this can be used to set a
 *  MdmPathDescriptor variable declared on the stack to the empty state.
 */
#define EMPTY_PATH_DESCRIPTOR {0, {0, {0, 0, 0, 0}}, {0}}



/** A structure to hold info about an OID.
 *
 * Each process needs to have their own array of these because
 * processes can (and does) load their handler functions at different
 * addresses.  So these pointers cannot be inside the MdmObjectNode, 
 * which is in shared memory.  If we are not using shared memory,
 * then defining the handler pointers outside the MdmObjectNode is
 * not necessary, but still will work.
 */
typedef struct 
{
   MdmObjectId oid;             /**< oid of this object. */
   const char *fullPath;        /**< full generic pathname of this OID */
   CmsRet (*rclHandlerFuncPreHook)(); /**< function pointer to RCL pre handler. */
   CmsRet (*rclHandlerFunc)(); /**< function pointer to RCL handler. */
   CmsRet (*rclHandlerFuncPostHook)(); /**< function pointer to RCL post handler. */
   CmsRet (*stlHandlerFunc)(); /**< function pointer to STL handler. */
   CmsRet (*stlHandlerFuncPostHook)(); /**< function pointer to STL post handler. */
} MdmOidInfoEntry;



/** This shmId value means its uninitialized */
#define UNINITIALIZED_SHM_ID  -1

#ifdef __cplusplus
extern "C" {
#endif

UBOOL8 cmsMdm_isInitialized(void);

/** Initialize the mdm layer.  This is the preferred init call because it is
 * more efficient.
 *
 * This function can be called multiple times, but only the first call
 * will have any effect.  This function must be called before any
 * mdm, obj, phl, lck, or mgm functions are called.
 *
 * @param eid       (IN) The CmsEntityId of the caller.
 * @param acc       (IN) MDM access flags (NDA_ACCESS_xxx)
 * @param msgHandle (IN) The message handle for the communications link
 *                       between the caller and the smd.  This msgHandle
 *                       was filled in by a previous call to cmsMsg_init().
 *                       This argument is required for all callers except smd.
 *                       If smd is the caller, this argument must be NULL.
 * @param shmId (IN/OUT) The shared memory key for the MDM.  This parameter
 *                       is used only if CMS_SHARED_MEM is defined.
 *                       If CMS_SHARED_MEM is defined and shmId is -1, the
 *                       cmsMdm_init() will create a shared memory region,
 *                       attach to it, and return the shmId;
 *                       otherwise, cmsMdm_init() will attach to the memory
 *                       region specified by shmId.
 * @return CmsRef enum.
 */
CmsRet cmsMdm_initWithAcc(CmsEntityId eid, UINT32 acc, void *msgHandle, SINT32 *shmId);


/** Initialize the mdm layer.  If possible, use cmsMdm_initWithAcc().
 *
 * This function can be called multiple times, but only the first call
 * will have any effect.  This function must be called before any
 * mdm, obj, phl, lck, or mgm functions are called.
 *
 * @param eid       (IN) The CmsEntityId of the caller.
 * @param msgHandle (IN) The message handle for the communications link
 *                       between the caller and the smd.  This msgHandle
 *                       was filled in by a previous call to cmsMsg_init().
 *                       This argument is required for all callers except smd.
 *                       If smd is the caller, this argument must be NULL.
 * @param shmId (IN/OUT) The shared memory key for the MDM.  This parameter
 *                       is used only if CMS_SHARED_MEM is defined.
 *                       If CMS_SHARED_MEM is defined and shmId is -1, the
 *                       cmsMdm_init() will create a shared memory region,
 *                       attach to it, and return the shmId;
 *                       otherwise, cmsMdm_init() will attach to the memory
 *                       region specified by shmId.
 * @return CmsRef enum.
 */
CmsRet cmsMdm_init(CmsEntityId eid, void *msgHandle, SINT32 *shmId);


/** Clean up the mdm layer.
 *
 * This function should only be called once before the application exits.
 */
void cmsMdm_cleanup(void);


/** Return the MdmNamePath for the full path.
 *
 * This is strictly a conversion function.  It does not verify that the
 * fullpath actually points to a valid or present object or parameter in the MDM.
 *
 * @param fullpath (IN) This must be a fully qualified object or parameter name,
 * @param pathDesc (OUT) User must allocate memory for the MdmPathDescriptor.  This
 *                       function will just fill it in.
 *
 * @return CmsRet enum.
 */
CmsRet cmsMdm_fullPathToPathDescriptor(const char *fullpath, MdmPathDescriptor *pathDesc);


/** Return fullpath given the MdmNamePath.
 *
 * This is strictly a conversion function.  It does not verify that the
 * pathDesc actually specifies a valid or present object or parameter in the MDM.
 *
 * @param pathDesc (IN) MdmPathDescriptor to convert.
 * @param fullpath (OUT) This will be a fully qualified object or parameter name.
 *                       the user is responsible for freeing the fullpath.
 * @return CmsRet enum.
 */
CmsRet cmsMdm_pathDescriptorToFullPath(const MdmPathDescriptor *pathDesc, char **fullpath);


/** Given a MdmPathDescriptor which references an object instance, return a
 *  fullpath which is suitable for use as an object reference in the data
 *  model, namely, the fullpath ends with the instance number and no trailing
 *  ".".
 *
 *  This function just calls cmsMdm_pathDescriptorToFullPath and strips out
 *  the trailing dot.
 *
 * @param pathDesc (IN) MdmPathDescriptor to convert.
 * @param fullpath (OUT) This is the fullpath with no end dot.  The user is
 *                       responsible for freeing the fullpath.
 * @return CmsRet enum.
 */
CmsRet cmsMdm_pathDescriptorToFullPathNoEndDot(const MdmPathDescriptor *pathDesc, char **fullpath);


/** Print the contents of iidStack to a global tmp buffer and
 * return a pointer to that buffer.
 *
 * This form of the iidStack function will work if you have
 * a single threaded program and are only printing out one iidStack
 * at a time.  See also mdm_dumpIidStackToBuf.
 *
 * @param iidStack (IN) iidStack to output.
 * @return pointer to buffer.
 */
char *cmsMdm_dumpIidStack(const InstanceIdStack *iidStack);


/** Print the contents of iidStack in the given buffer.
 *
 * This form of the dumpIidStack function is useful if you want
 * to specify a buffer larger than the default buffer or if
 * you want to print two iidStacks on the same printf line or
 * if you are have a multi-threaded program.
 *
 * @param iidStack (IN) iidStack to output.
 * @param buf (OUT) buffer to put the formatted iidstack into.
 * @param len (IN) length of buffer.  If len is not long enough to hold the entire
 *                 iidStack, the last digit will be 'z', which means truncation
 *                 occurred.
 * @return pointer to buffer.
 */
char *cmsMdm_dumpIidStackToBuf(const InstanceIdStack *iidStack,
                               char *buf,
                               UINT32 len);


/** Compare the two iidstacks similar to the way strcmp works.
 *
 * This might be only used by mdm.c, but it does not hurt to
 * export a generally useful function such as this.
 *
 * @param iidStack1 (IN) pointer to instanceIdStack.
 * @param iidStack2 (IN) pointer to instanceIdStack.
 * @return -1 if iidStack1 is "less than" iidStack2, 0 if they
 *            are equal, and 1 if iidStack1 is "greater than" 
 *            iidStack2.
 */
SINT32 cmsMdm_compareIidStacks(const InstanceIdStack *iidStack1,
                               const InstanceIdStack *iidStack2);


/** Compare the two iidstacks similar to the way strncmp works.
 *
 * This might be only used by mdm.c, but it does not hurt to
 * export a generally useful function such as this.
 *
 * @param iidStack1 (IN) pointer to instanceIdStack.
 * @param iidStack2 (IN) pointer to instanceIdStack.
 * @param n         (IN) number of stack levels to compare.
 * @return -1 if iidStack1 is "less than" iidStack2, 0 if they
 *            are equal, and 1 if iidStack1 is "greater than" 
 *            iidStack2.
 */
SINT32 cmsMdm_compareIidStacksToDepth(const InstanceIdStack *iidStack1,
                                      const InstanceIdStack *iidStack2,
                                      UINT32 n);


 /*
 * Check if the instance id's in the fullpath actually exist in the MDM
 *
 * @param pathDesc (IN) The full path descriptor to check against.
 * 
 * @return TRUE if the fullpath does exist in the MDM, otherwise,
 *         FALSE.
 */
UBOOL8 cmsMdm_isPathDescriptorExist(const MdmPathDescriptor *pathDesc);


/*
 * Return TRUE if the root of the data model is Device. i.e. the MDM
 * currently contains the Pure TR181 data model (not IGD or Hybrid).
 * Note the caller of this function must be already attached to the MDM.
 *
 * @return TRUE if the root of the data model is Device.
 *         FALSE is the root of the data model is InternetGatewayDevice (Legacy98 or Hybrid)
 */
UBOOL8 cmsMdm_isDataModelDevice2(void);


/** Get pointer to the beginning of all OID info entries.
 *
 * @param numEntries (OUT) total number of entries in the array.
 * @return pointer to the beginning of the OID info array.  Do NOT free it.
 */
const MdmOidInfoEntry *cmsMdm_getAllOidInfoEntries(UINT32 *numEntries);

/** Get pointer to the requested OID.
 *
 * @param oid (IN) The requested OID.
 * @return pointer to the OID info entry; NULL if not found.
 */
const MdmOidInfoEntry *cmsMdm_getOidInfoEntry(UINT16 oid);

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* __CMS_MDM_H__ */
