/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2024:DUAL/GPL:standard

   Unless you and Broadcom execute a separate written software license
   agreement governing use of this software, this software is licensed
   to you under the terms of the GNU General Public License version 2
   (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
   with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

   Not withstanding the above, under no circumstances may you combine
   this software in any way with any other Broadcom software provided
   under a license other than the GPL, without Broadcom's express prior
   written consent.

   :>
*/


/****************************************************************************
 ****************************************************************************
 *  File Name     :  merlin8_phy1_version.h                                   *
 *  Created On    :  4/27/2018                                              *
 *  Created By    :  Kiran Divakar                                          *
 *  Description   :  Declare the API version number                         *
 *  Revision      :                                                     *
 *                                                                          *
 ****************************************************************************
 ****************************************************************************/

#ifndef API_VERSION_H
#define API_VERSION_H

/*! @file
 *  @brief Declare the API version number
 */

#define API_MAJOR_VERSION 0x01
#define API_MINOR_VERSION 0x00
#define API_PATCH_VERSION 0x00
#define API_RC_VERSION    0x01

#define API_VERSION  ( (((uint32_t)API_MAJOR_VERSION) << 24)    \
                     | (((uint32_t)API_MINOR_VERSION) << 16)    \
                     | (((uint32_t)API_PATCH_VERSION) << 8)     \
                     | (((uint32_t)API_RC_VERSION)    << 0) )

#endif
