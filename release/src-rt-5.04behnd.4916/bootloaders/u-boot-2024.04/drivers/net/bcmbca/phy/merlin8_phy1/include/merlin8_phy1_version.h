// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved
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
#define API_RC_VERSION 0x08

#define API_VERSION  ( (((uint32_t)API_MAJOR_VERSION) << 24)    \
                     | (((uint32_t)API_MINOR_VERSION) << 16)    \
                     | (((uint32_t)API_PATCH_VERSION) << 8)     \
                     | (((uint32_t)API_RC_VERSION)    << 0) )

#endif
