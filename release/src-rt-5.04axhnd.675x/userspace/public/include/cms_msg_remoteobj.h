/***********************************************************************
 *
 * Copyright (c) 2019  Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2019:DUAL/GPL:standard
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
 * :>
 *
 ************************************************************************/

/*!\file cms_msg_remoteobj.h
 * \brief This file contains the message bodies used in CMS REMOTE_OBJ msg types.
 */

#ifndef CMS_MSG_REMOTEOBJ_H
#define CMS_MSG_REMOTEOBJ_H

#include "cms_mdm.h"

typedef struct
{
    MdmObjectId oid;
    InstanceIdStack iidStack;
    UINT32 flags;
} RemoteObjGetRequest;

typedef struct
{
    void *objData;
} RemoteObjGetResponse;

typedef struct
{
    MdmObjectId oid;
    InstanceIdStack parentIidStack;
    InstanceIdStack iidStack;
    UINT32 flags;
} RemoteObjGetNextRequest;

typedef struct
{
    void *objData;
    InstanceIdStack iidStack;
} RemoteObjGetNextResponse;

typedef struct
{
    UINT32 flags;
    char fullpath[MDM_SINGLE_FULLPATH_BUFLEN];
} RemoteObjAddInstanceRequest;

typedef struct
{
    UINT32 newInstanceId;
} RemoteObjAddInstanceResponse;

typedef struct
{
    UINT32 flags;
    char fullpath[MDM_SINGLE_FULLPATH_BUFLEN];
} RemoteObjDelInstanceRequest;

typedef struct
{
    void *objData;
    InstanceIdStack iidStack;
    UINT32 flags;
} RemoteObjSetRequest;

/* This structure is followed by an array of MdmPathDescriptor. */
typedef struct
{
    UBOOL8 nextLevelOnly;
    UINT32 flags;
    SINT32 numEntries;
} RemoteObjGetParamsRequest;

/* This structure is followed by a serialized array of parameter values. */
typedef struct
{
    SINT32 numEntries;
    UINT32 size;
} RemoteObjGetParamsResponse;

/* This structure is followed by an array of serialized parameter set request. */
typedef struct
{
    SINT32 numEntries;
    UINT32 size;
    UINT32 flags;
} RemoteObjSetParamsRequest;

/* This structure is followed by an array of UINT32 indicating status of
 * each request in order.
 */
typedef struct
{
    SINT32 numEntries;
} RemoteObjSetParamsResponse;

/* This structure is followed by an array of MdmPathDescriptor. */
typedef struct
{
    UBOOL8 nextLevelOnly;
    SINT32 numEntries;
    UINT32 flags;
} RemoteObjGetParamAttributesRequest;

/* This structure is followed by an array of
 * {char *fullPath, UBOOL8 accessBitMaskChange, UBOOL16 accessBitMask,
 *  UBOOL8 notificationChange, UBOOL16 notification
 * }.
 */
typedef struct
{
    SINT32 numEntries;
    SINT32 size;
} RemoteObjGetParamAttributesResponse;

/* This structure is followed by an array of PhlSetParamAttr_t. */
typedef struct
{
    SINT32 numEntries;
} RemoteObjSetParamAttributesRequest;

typedef struct
{
    MdmObjectId ancestorOid;
    MdmObjectId decendentOid;
    InstanceIdStack iidStack;
    UINT32 flags;
} RemoteObjGetAncestorRequest;

typedef struct
{
    void *objData;
    InstanceIdStack iidStack;
} RemoteObjGetAncestorResponse;

typedef struct
{
    char fullPath[CMS_MAX_FULLPATH_LENGTH];
    UBOOL8 nextLevelOnly;
    UINT32 flags;   
} RemoteObjGetParamNamesRequest;

/* This structure is followed by an array of {char *fullPath, UBOOL8 writable}. */
typedef struct
{
    SINT32 numEntries;
    UINT32 size;
} RemoteObjGetParamNamesResponse;

typedef struct
{
    MdmPathDescriptor pathDesc;
} RemoteObjPathDesc;

#endif /* CMS_MSG_REMOTEOBJ_H */
