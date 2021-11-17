/***********************************************************************
 *
 * Copyright (c) 2006-2007  Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2006-2007:DUAL/GPL:standard
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
 * :>
 *
 ************************************************************************/

#ifndef __CMS_MSG_HOMEPLUG_H__
#define __CMS_MSG_HOMEPLUG_H__

#include "cms.h"
#include "cms_msg.h"

/*!\file cms_msg_homeplug.h
 * \brief defines for specific HomePlug messages.
 *
 * HomePlug messages are from 0x10002700-0x100027FF
 *
 * In cms_msg.h are defined HomePlug messages commonly used throughout the 
 * system. This file contains HomePlug messages with a more restricted 
 * scope (typically, homeplugd and another entity).
 */

/*!\enum CmsHomePlugMsgType
 * \brief  Enumeration of possible message types
 *
 */
typedef enum 
{
  CMS_MSG_HOMEPLUG_PASSWORD_SET          = 0x10002704, /**< set Password */
  CMS_MSG_HOMEPLUG_ALIAS_SET             = 0x10002705, /**< set Alias */
  CMS_MSG_HOMEPLUG_LOGICALNET_SET        = 0x10002706, /**< set LogicalNetwork */
  CMS_MSG_HOMEPLUG_DIAGINTERVAL_SET      = 0x10002707  /**< set X_BROADCOM_COM_DiagPeriodicInterval */

} CmsHomePlugMsgType;

/** Lengths for the messages parameters.
 *
 */
#define CMS_MSG_HOMEPLUG_PASSWORD_LENGTH        BUFLEN_32
#define CMS_MSG_HOMEPLUG_ALIAS_LENGTH           BUFLEN_64
#define CMS_MSG_HOMEPLUG_LOGICALNET_LENGTH      BUFLEN_64

/** Data body for CMS_MSG_HOMEPLUG_PASS_SET message type.
 *
 */
typedef struct
{
   char password[CMS_MSG_HOMEPLUG_PASSWORD_LENGTH];
} HomePlugPasswordSetMsgBody;

/** Data body for CMS_MSG_HOMEPLUG_ALIAS_SET message type.
 *
 */
typedef struct
{
   char alias[CMS_MSG_HOMEPLUG_ALIAS_LENGTH];
} HomePlugAliasSetMsgBody;

/** Data body for CMS_MSG_HOMEPLUG_LOGICALNET_SET message type.
 *
 */
typedef struct
{
   char logicalNetwork[CMS_MSG_HOMEPLUG_LOGICALNET_LENGTH];
} HomePlugLogicalNetworkSetMsgBody;

/** Data body for CMS_MSG_HOMEPLUG_DIAGINTERVAL_SET message type.
 *
 */
typedef struct
{
   UINT32 X_BROADCOM_COM_DiagPeriodicInterval;
} HomePlugDiagPeriodicIntervalSetMsgBody;

#endif /* __CMS_MSG_HOMEPLUG_H__ */
