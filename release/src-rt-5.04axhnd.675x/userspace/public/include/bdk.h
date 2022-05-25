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


#ifndef __BDK_H__
#define __BDK_H__

/*!\file bdk.h
 * \brief Global BDK defines.  Only very simple data types and definitions
 *        should be in this file.  Try to push defines closer to their
 *        specific subsystems.
 */


// Max length of a component name.
#define BDK_COMP_NAME_LEN    31

/** These are the official component names in BDK. */
#define BDK_COMP_DEVINFO        "devinfo"
#define BDK_COMP_DIAG           "diag"
#define BDK_COMP_DSL            "dsl"
#define BDK_COMP_GPON           "gpon"
#define BDK_COMP_EPON           "epon"
#define BDK_COMP_WIFI           "wifi"
#define BDK_COMP_VOICE          "voice"
#define BDK_COMP_STORAGE        "storage"
#define BDK_COMP_TR69           "tr69"
#define BDK_COMP_USP            "usp"
#define BDK_COMP_SYSMGMT        "sysmgmt"
#define BDK_COMP_OPENPLAT       "openplat"
#define BDK_COMP_SYS_DIRECTORY  "sys_directory"
#define BDK_COMP_WRTGPONAGT     "wrtgponagt"


// wrtsysmgmt is virtual component wrap uci access, it sits on the U-Bus in OpenWrt environments.
#define BDK_COMP_WRTSYSMGMT       "wrtsysmgmt"

// Firewalld is not a component, rather it is an app that runs inside the
// sysmgmt component.  But it is serving requests on DBus.
#define BDK_APP_FIREWALLD       "firewalld"

// apibdk_sd is not a component, rather it is an app that sits on the U-Bus in OpenWrt environments.
#define BDK_APP_APIBDK_SD       "apibdk_sd"

// openwrt_wifi_agent is not a component, rather it is an app that sits on the U-Bus in OpenWrt environments.
#define BDK_APP_WRTWIFIAGT       "wrtwifiagt"

// openwrt_dsl_agent is not a component, rather it is an app that sits on the U-Bus in OpenWrt environments.
#define BDK_APP_WRTDSLAGT       "wrtdslagt"

// sysdirctl is not a component, but just a test app.
#define BDK_COMP_SYSDIRCTL      "sysdirctl"

// If we need to give a component name while in CMS mode, use this:
#define BDK_COMP_CMS_CLASSIC    "CMS_Classic"

// openwrt_tr69_agent is not a component, rather it is an app that sits on the U-Bus in OpenWrt environments.
#define BDK_APP_WRTTR69AGT       "wrttr69agt"

// openwrt_voice_agent is not a component, rather it is an app that sits on the U-Bus in OpenWrt environments.
#define BDK_APP_WRTVOIPAGT       "wrtvoipagt"
#endif /* BDK_H */
