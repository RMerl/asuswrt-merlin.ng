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


/*****************************************************************************
*    Description:
*
*      BDK DBUS header file.
*
* This is also used in UBus compiles, so a more appropriate name for this
* file would be bdk_zbus.h (or maybe just fold it all into bdk.h?)
*
*****************************************************************************/
#ifndef BDK_DBUS_H
#define BDK_DBUS_H

/* Global D-Bus defines */
/* These are also defined in BEEP, but no big deal to repeat it here */
#define SYSTEM_DBUS_BUS_NAME                  "org.freedesktop.DBus"
#define SYSTEM_DBUS_INTERFACE_NAME            "org.freedesktop.DBus"
#define SYSTEM_DBUS_OBJECT_PATH               "/org/freedesktop/DBus"


/* D-Bus related defines for each of the BDK Component Management Daemons.
 */
#define DEVINFO_MD_BUS_NAME            "com.broadcom.devinfo_md"
#define DEVINFO_MD_INTERFACE_NAME      "com.broadcom.devinfo_md"
#define DEVINFO_MD_OBJECT_PATH         "/com/broadcom/devinfo_md"

#define DIAG_MD_BUS_NAME               "com.broadcom.diag_md"
#define DIAG_MD_INTERFACE_NAME         "com.broadcom.diag_md"
#define DIAG_MD_OBJECT_PATH            "/com/broadcom/diag_md"

#define DSL_MD_BUS_NAME                "com.broadcom.dsl_md"
#define DSL_MD_INTERFACE_NAME          "com.broadcom.dsl_md"
#define DSL_MD_OBJECT_PATH             "/com/broadcom/dsl_md"

#define GPON_MD_BUS_NAME               "com.broadcom.gpon_md"
#define GPON_MD_INTERFACE_NAME         "com.broadcom.gpon_md"
#define GPON_MD_OBJECT_PATH            "/com/broadcom/gpon_md"

#define EPON_MD_BUS_NAME               "com.broadcom.epon_md"
#define EPON_MD_INTERFACE_NAME         "com.broadcom.epon_md"
#define EPON_MD_OBJECT_PATH            "/com/broadcom/epon_md"

#define WIFI_MD_BUS_NAME               "com.broadcom.wifi_md"
#define WIFI_MD_INTERFACE_NAME         "com.broadcom.wifi_md"
#define WIFI_MD_OBJECT_PATH            "/com/broadcom/wifi_md"

#define VOICE_MD_BUS_NAME              "com.broadcom.voice_md"
#define VOICE_MD_INTERFACE_NAME        "com.broadcom.voice_md"
#define VOICE_MD_OBJECT_PATH           "/com/broadcom/voice_md"

#define STORAGE_MD_BUS_NAME            "com.broadcom.storage_md"
#define STORAGE_MD_INTERFACE_NAME      "com.broadcom.storage_md"
#define STORAGE_MD_OBJECT_PATH         "/com/broadcom/storage_md"

#define TR69_MD_BUS_NAME               "com.broadcom.tr69_md"
#define TR69_MD_INTERFACE_NAME         "com.broadcom.tr69_md"
#define TR69_MD_OBJECT_PATH            "/com/broadcom/tr69_md"

#define USP_MD_BUS_NAME                "com.broadcom.usp_md"
#define USP_MD_INTERFACE_NAME          "com.broadcom.usp_md"
#define USP_MD_OBJECT_PATH             "/com/broadcom/usp_md"

#define SYSMGMT_MD_BUS_NAME            "com.broadcom.sysmgmt_md"
#define SYSMGMT_MD_INTERFACE_NAME      "com.broadcom.sysmgmt_md"
#define SYSMGMT_MD_OBJECT_PATH         "/com/broadcom/sysmgmt_md"

/* non-blocking bus server in sysmgmt */
#define SYSMGMT_NB_BUS_SVR_BUS_NAME        "com.broadcom.sysmgmt_nb_bus_svr"
#define SYSMGMT_NB_BUS_SVR_INTERFACE_NAME  "com.broadcom.sysmgmt_nb_bus_svr"
#define SYSMGMT_NB_BUS_SVR_OBJECT_PATH     "/com/broadcom/sysmgmt_nb_bus_svr"


#define OPENPLAT_MD_BUS_NAME             "com.broadcom.openplat_md"
#define OPENPLAT_MD_STRPROTO_INTERFACE   "com.broadcom.openplat_md_strproto"
#define OPENPLAT_MD_OBJECT_PATH          "/com/broadcom/openplat_md"


#define SYS_DIRECTORY_BUS_NAME            "com.broadcom.sys_directory"
#define SYS_DIRECTORY_INTERFACE_NAME      "com.broadcom.sys_directory"
#define SYS_DIRECTORY_OBJECT_PATH         "/com/broadcom/sys_directory"

#define SYSDIRCTL_BUS_NAME             "com.broadcom.sysdirctl"
#define SYSDIRCTL_INTERFACE_NAME       "com.broadcom.sysdirctl"
#define SYSDIRCTL_OBJECT_PATH          "/com/broadcom/sysdirctl"


/* Firewalld is not a component, but a daemon that attaches to the DBus
 * inside the sysmgmt component. */
#define FIREWALLD_BUS_NAME             "com.broadcom.FirewallService"
#define FIREWALLD_OBJECT_PATH          "/com/broadcom/FirewallService"
#define FIREWALLD_INTERFACE            "com.broadcom.FirewallService"

#define USP_MD_BUS_NAME                "com.broadcom.usp_md"
#define USP_MD_INTERFACE_NAME          "com.broadcom.usp_md"
#define USP_MD_OBJECT_PATH             "/com/broadcom/usp_md"

/* OpenWRT wrtsysmgmt is virtual mdm component. */
#define WRTSYSMGMT_BUS_NAME              "neutron.wrtsysmgmt_md"
#define WRTSYSMGMT_OBJECT_PATH           "neutron.wrtsysmgmt_md"
#define WRTSYSMGMT_INTERFACE_NAME        "neutron.wrtsysmgmt_md"

/* OpenWRT GPON Agent. */
#define WRT_GPONAGT_BUS_NAME            "com.broadcom.wrtgponagt"
#define WRT_GPONAGT_INTERFACE_NAME      "com.broadcom.wrtgponagt"
#define WRT_GPONAGT_OBJECT_PATH         "/com/broadcom/wrtgponagt"

/* OpenWRT VOIP Agent. */
#define WRT_VOIPAGT_BUS_NAME            "com.broadcom.wrtvoipagt"
#define WRT_VOIPAGT_INTERFACE_NAME      "com.broadcom.wrtvoipagt"
#define WRT_VOIPAGT_OBJECT_PATH         "/com/broadcom/wrtvoipagt"


#define APIBDK_SD_BUS_NAME             "neutron.apibdk"
#define APIBDK_SD_OBJECT_PATH          "neutron.apibdk"
/*no interface concept in ubus context. anyway, define it here */
#define APIBDK_SD_INTERFACE            "neutron.apibdk"

#define AUDIO_MD_BUS_NAME               "com.broadcom.audio_md"
#define AUDIO_MD_INTERFACE_NAME         "com.broadcom.audio_md"
#define AUDIO_MD_OBJECT_PATH            "/com/broadcom/audio_md"

#endif /* BDK_DBUS_H */
