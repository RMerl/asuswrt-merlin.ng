/***********************************************************************
 *
 * Copyright (c) 2019  Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2019:DUAL/GPL:standard
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

// sysmgmt_nb_bus_svr is a non-blocking bus server that helps the main sysmgmt_md.
// it runs in the sysmgmt component.
#define BDK_APP_SYSMGMT_NB       "sysmgmt_nb"

// apibdk_sd is not a component, rather it is an app that sits on the U-Bus in OpenWrt environments.
#define BDK_APP_APIBDK_SD       "apibdk_sd"

// sysdirctl is not a component, but just a test app.
#define BDK_COMP_SYSDIRCTL      "sysdirctl"

// If we need to give a component name while in CMS mode, use this:
#define BDK_COMP_CMS_CLASSIC    "CMS_Classic"

// openwrt_voice_agent is not a component, rather it is an app that sits on the U-Bus in OpenWrt environments.
#define BDK_APP_WRTVOIPAGT      "wrtvoipagt"

// audio component
#define BDK_COMP_AUDIO          "audio_md"
#endif /* BDK_H */
