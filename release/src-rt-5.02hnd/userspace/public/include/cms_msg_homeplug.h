/***********************************************************************
 *
 * Copyright (c) 2006-2007  Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2006-2007:DUAL/GPL:standard
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
