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

#ifndef __CMS_H__
#define __CMS_H__

/*!\file cms.h
 * \brief Header file containing common and constant definitions for
 *        the CPE Management System (CMS).  Parameters which may change
 *        depending on vendor preference or board configuration are located
 *        in cms_params.h (which is included by this file at the bottom.)
 */

#include "number_defs.h"
#include "os_defs.h"
#include "cms_version.h"


#include "cms_retcodes.h"

/** Check if the CmsRet code is either SUCCESS or SUCCESS_REBOOT_REQUIRED.
 * CMSRET_SUCCESS_REBOOT is returned in two scenarios:
 * 1. During a set, the RCL handler function has accepted the new value but
 *    is unable to change the system configuration at run-time, so a reboot
 *    is needed before the change can take effect.
 * 2. During a AddObject or DeleteObject.  The MDM object tree has been
 *    successfully updated, but the RCL handler function is unable to change
 *    the system configuration at run-time, so a reboot is needed before the
 *    change can take effect.
 * Be very careful if you add any other return codes to this macro.
 * Make sure you understand what you are doing.  Specifically,
 * do not add CMSRET_SUCCESS_UNRECOGNIZED_DATA_IGNORED or
 * CMSRET_SUCCESS_OBJECT_UNCHANGED to this macro.  These return codes are
 * used in different contexts.
 */
#define IS_CMSRET_A_SUCCESS_VARIANT(r) (((r) == CMSRET_SUCCESS) || \
                                        ((r) == CMSRET_SUCCESS_REBOOT_REQUIRED) || \
                                        ((r) == CMSRET_SUCCESS_APPLY_NOT_COMPLETE))

/** Check if the CmsRet code is a TR69c recognized value.
 */
#define IS_CMSRET_A_TR69C_VARIANT(r) (((r) == CMSRET_SUCCESS) ||        \
                                      (((r) >= 9000) && ((r) < 9800)))



#ifndef TRUE
/** TRUE = 1
 */
#define TRUE  1
#endif

#ifndef FALSE
/** FALSE = 0
 */
#define FALSE 0
#endif

/** Maximum value for a UINT64 */
#define MAX_UINT64 18446744073709551615ULL

/** Maximum value for a SINT64 */
#define MAX_SINT64 9223372036854775807LL

/** Minimum value for a SINT64 */
#define MIN_SINT64 (-1 * MAX_SINT64 - 1)

/** Maximum value for a UINT32 */
#define MAX_UINT32 4294967295U

/** Maximum value for a SINT32 */
#define MAX_SINT32 2147483647

/** Minimum value for a SINT32 */
#define MIN_SINT32 (-2147483648)

/** Maximum value for a UINT16 */
#define MAX_UINT16  65535

/** Maximum value for a SINT16 */
#define MAX_SINT16  32767

/** Minimum value for a SINT16 */
#define MIN_SINT16  (-32768)


/**
 * This is common used string length types.
 */
#define BUFLEN_4        4     //!< buffer length 4
#define BUFLEN_8        8     //!< buffer length 8
#define BUFLEN_16       16    //!< buffer length 16
#define BUFLEN_18       18    //!< buffer length 18 -- for ppp session id
#define BUFLEN_24       24    //!< buffer length 24 -- mostly for password
#define BUFLEN_32       32    //!< buffer length 32
#define BUFLEN_40       40    //!< buffer length 40
#define BUFLEN_48       48    //!< buffer length 48
#define BUFLEN_64       64    //!< buffer length 64
#define BUFLEN_80       80    //!< buffer length 80
#define BUFLEN_128      128   //!< buffer length 128
#define BUFLEN_256      256   //!< buffer length 256
#define BUFLEN_264      264   //!< buffer length 264
#define BUFLEN_512      512   //!< buffer length 512
#define BUFLEN_1024     1024  //!< buffer length 1024

#define IIDSTACK_BUF_LEN  40  //!< good length to use for mdm_dumpIidStack
#define MAC_ADDR_LEN    6     //!< Mac address len in an array of 6 bytes
#define MAC_STR_LEN     17    //!< Mac String len with ":". eg: xx:xx:xx:xx:xx:xx
#define VPI_MIN         0     //!< VPI min
#define VPI_MAX         255   //!< VPI max
#define VCI_MIN         32    //!< VCI min
#define VCI_MAX         65535 //!< VCI max

#define PPP_CONNECT_ERROR_REASON_LEN 48

#define CMS_IFNAME_LENGTH  BUFLEN_32   //!< broadcom interface name length
#define CMS_MAX_ACS_URL_LENGTH   260   //!< max acs url from dhcp server, specified in TR-181 as max length 256
#define CMS_MAX_ACS_PROVISIONING_CODE_LENGTH 68  //!< max acs provisioning code, TR-181 max length is 64

#define CMS_AFTR_NAME_LENGTH   256     //!< max aftr name from dhcpv6 server
#define CMS_DEV2_RA_PREFIX_LEN   288   //!< buffer length of prefix in RA object in TR181

#ifdef SUPPORT_IPV6
#define CMS_IPADDR_LENGTH  46          //!< IP address length to hold IPv6 in CIDR notation (match INET6_ADDRSTRLEN)
#else
#define CMS_IPADDR_LENGTH  BUFLEN_16   //!< IP address length to hold IPv4 in ASCII
#endif /* SUPPORT_IPV6 */



/** Some defines for selecting which address family (IPv4 or IPv6) we want.
 *  Note: are bits for use in bitmasks (not integers)
 */
#define CMS_AF_SELECT_IPV4       0x0001
#define CMS_AF_SELECT_IPV6       0x0002

#define CMS_AF_SELECT_IPVX       (CMS_AF_SELECT_IPV4|CMS_AF_SELECT_IPV6)

#define IS_CMS_AF_ALLOW_IPV4 (ipvx)  ((ipvx) & CMS_AF_SELECT_IPV4)
#define IS_CMS_AF_ALLOW_IPV6 (ipvx)  ((ipvx) & CMS_AF_SELECT_IPV6)



#define CMS_MAX_DEFAULT_GATEWAY     8  //!< max default gateways allowed in L3 X_BROADCOM_COM_DefaultConnectionServices
#define CMS_MAX_DNSIFNAME           8  //!< max dns wan interface names in X_BROADCOM_Networking.DNSIfName
#define CMS_MAX_ACTIVE_DNS_IP       4  //!< max active dns ip (in resolv.conf)

#ifdef DMP_X_BROADCOM_COM_GPONRG_OMCI_FULL_1
#define CMS_MAX_GPONWAN_INTF        1  //!< max gpon wan layer 2 interfaces for full omci GponRg
#else
#define CMS_MAX_GPONWAN_INTF        0  //!< No gpon at all, so no GPONWAN intf
#endif

#define CMS_MAX_ETHLAG_INTF         2        //!< max ethernet LAG interfaces supported )
#define DEFAULT_ETHLAG_0            "bond0"  //!< first default ethlag interface name
#define DEFAULT_ETHLAG_1            "bond1"  //!< first default ethlag interface name

/**
 * Values for network protocol
 */
#define PROTO_PPPOE        0  //!< PPPoE protocol
#define PROTO_PPPOA        1  //!< PPPoA protocol
#define PROTO_MER          2  //!< MER protocol
#define PROTO_BRIDGE       3  //!< bridge protocol
#define PROTO_PPPOE_RELA   4  //!< PPPoE relay protocol
#define PROTO_IPOA         5  //!< ip over atm protocol
#define PROTO_IPOWAN       6  //!< ip over wan protocol
#define PROTO_NONE         10 //!< invalid protocol

#define IFC_WAN_MAX        16 //!< Max WAN Connection in the system.
#define IFC_VLAN_MAX       16  //!< Max VLAN on single base WAN interface.

/*!\enum WanIfcType
 * \brief Enumerated values of WAN interface types.
 */
typedef enum {
   WAN_IFC_ATM=0,       /**< ATM */
   WAN_IFC_PPPOA=1,     /**< PPPoA */
   WAN_IFC_IPOA=2,      /**< IPoA */
   WAN_IFC_ETH=3,       /**< Eth */
   WAN_IFC_PTM=4,       /**< Ptm */
   WAN_IFC_WIFI=5,      /**< Wifi */
   WAN_IFC_ETHLAG=7,    /**< EthLAG */
   WAN_IFC_NONE=10      /**< undefined/invalid */
} WanIfcType;

#ifdef NOT_USED_AND_USE_CMSWANCONNECTIONTYPE_BELOW
/******************* NOTE:  DO NOT USE WanProtocal. USE CmsWanConnetctionType Below !!!!! ***********
 * !\enum WanProtocol
 * \brief Enumerated values of WAN connection protocols.
 * This should be used to replace the same set of defines in cms.h
 */
typedef enum {
   WAN_PROTO_PPPOE=0,   /**< PPPoE */
   WAN_PROTO_PPPOA=1,   /** < PPPoA */
   WAN_PROTO_MER=2,     /**< static or dynamic mer */
   WAN_PROTO_BRIDGE=3,  /**< bridge */
   WAN_PROTO_PPPOE_RELAY=4,  /**< pppoe relay */
   WAN_PROTO_IPOA=5,     /**< IPoA */
   WAN_PROTO_IPOWAN=6,   /**< IP over Wan only when SUPPORT ETHWAN */
   WAN_PROTO_NONE=10     /**< no proto found/defined/invalid */
} WanProtocol;
#endif

/* try to match with the above old defines  PROTO_PPPOE=0 PPPOA=1, MER=2, BRIDGE=3 thing
 * so that no html changes need  */
typedef enum

{
   CMS_WAN_TYPE_PPPOE               = 0,
   CMS_WAN_TYPE_PPPOA               = 1,
   CMS_WAN_TYPE_DYNAMIC_IPOE        = 2,
   CMS_WAN_TYPE_BRIDGE              = 3,
   CMS_WAN_TYPE_PPPOE_RELAY         = 4,
   CMS_WAN_TYPE_IPOA                = 5,
   CMS_WAN_TYPE_STATIC_IPOE         = 6,

   CMS_WAN_TYPE_STATIC_ETHERNET_IP  = 10,
   CMS_WAN_TYPE_DYNAMIC_ETHERNET_IP = 11,
   CMS_WAN_TYPE_ETHERNET_PPPOE      = 12,
   CMS_WAN_TYPE_ETHERNET_BRIDGE     = 13,
   CMS_WAN_TYPE_UNDEFINED           = 99
} CmsWanConnectionType;



#define BRIDGE_PROTO_STR      "Bridge"
#define IPOA_PROTO_STR        "IPoA"
#define IPOE_PROTO_STR        "IPoE"
#define PPPOE_PROTO_STR       "PPPoE"
#define PPPOA_PROTO_STR       "PPPoA"
#define PPPOL2TP_PROTO_STR    "PPPoL2tp"
#define IPOW_PROTO_STR        "IPoW"

#define DSL_IFC_STR           "dsl"
#define ETH_IFC_STR           "eth"
#define USB_IFC_STR           "usb"
#define WLAN_IFC_STR          "wl"
#define MOCA_IFC_STR          "moca"
#define HOMEPLUG_IFC_STR      "plc"
#define GPON_IFC_STR          "veip"
#define GPON_WAN_IF_NAME      "veip0"
#define ETHLAG_IFC_STR        "bond"

#define CMS_EPON_IF_NAME_CFG   "/tmp/cmsVeipIf.conf"

static inline char *epon_veip_name(void)
    {
    FILE* f = NULL;
    static char ifName[BUFLEN_16]="veip0";

    f = fopen(CMS_EPON_IF_NAME_CFG, "r");
    if (f != NULL)
       {
        if (fscanf(f, "%s", ifName) != 1)
            printf("fscanf error!!");
        fclose(f);
        }
    return ifName;
    }

#ifdef EPON_SFU
#define EPON_IFC_STR          "epon"
#define EPON_WAN_IF_NAME      "epon0"
#else
#define EPON_IFC_STR          "veip"
#define EPON_WAN_IF_NAME      (epon_veip_name())
#endif

#ifdef BRCM_PKTCBL_SUPPORT
#define EPON_VOICE_IFC_STR     "veip"
#define EPON_VOICE_WAN_IF_NAME "veip0.1"
#define EPON_EPTA_WAN_IF_NAME  "veip0.2"
#define EPON_NTP_DEFAULT_TIMEZONE  "Beijing"

typedef enum
{
   PKTCBL_WAN_EMTA = 1,
   PKTCBL_WAN_EPTA = 2
}PKTCBL_WAN_TYPE;
#endif

#if (DMP_X_BROADCOM_COM_MULTIPLE_TR69C_SUPPORT_1 == 2)
#if defined(BRCM_PKTCBL_SUPPORT)
#define TR69C_1_ASSOCIATED_WAN_IF_NAME   EPON_VOICE_WAN_IF_NAME
#define TR69C_2_ASSOCIATED_WAN_IF_NAME   EPON_EPTA_WAN_IF_NAME
#else
#define TR69C_1_ASSOCIATED_WAN_IF_NAME   "veip0.1" /* default */
#define TR69C_2_ASSOCIATED_WAN_IF_NAME   "veip0.2" /* default */
#endif // defined(BRCM_PKTCBL_SUPPORT)
#endif // (DMP_X_BROADCOM_COM_MULTIPLE_TR69C_SUPPORT_1 == 2)

#define MULTI_TR69C_CONFIG_INDEX_1  "1"
#define MULTI_TR69C_CONFIG_INDEX_2  "2"



#define CELLULAR_IFC_STR      "rmnet"
#define ATM_IFC_STR           "atm"
#define PTM_IFC_STR           "ptm"
#define BRIDGE_IFC_STR        "br"
#define IPOA_IFC_STR          "ipoa"
#define IPOE_IFC_STR          "ipoe"
#define PPP_IFC_STR           "ppp"
#define PPPOE_IFC_STR         "pppoe"
#define PPPOA_IFC_STR         "pppoa"
#define IPA_IFC_STR           "ipa"
#define BRIDGE_SMUX_STR       "bridge"

/* for interface group with routed pvc */
#define RT_TABLES_BASE	200

typedef enum
{
   ATM=0,           /**< WanDev is used for DSL ATM  */
   PTM=1,           /**< WanDev is used for DSL PTM  */
   Ethernet=2,      /**< WanDev is used for Ethernet  */
   EPONLINK=3,          /**< WanDev is used for EPON  */
   GPONLINK=4           /**< WanDev is used for GPON  */
}WanLinkType;


typedef enum
{
   CMS_CONNECTION_MODE_DEFAULT=0,      /**< Default connection mdoe - single wan service over 1 connection */
   CMS_CONNECTION_MODE_VLANMUX=1,      /**< Vlan mux connection mdoe - multiple vlan service over 1 connection */
   CMS_CONNECTION_MODE_MSC=2,          /**< MSC connection mdoe - multiple wan service over 1 connection */
} ConnectionModeType;


typedef enum
{
   ATM_EOA=0,        /**< WanDev is used for DSL ATM  */
   ATM_IPOA=1,       /**< WanDev is used for DSL ATM IPOA */
   ATM_PPPOA=2,      /**< WanDev is used for DSL ATM PPPoA */
   PTM_EOA=3         /**< WanDev is used for DSL PTM  */

}Layer2IfNameType;


typedef enum
{
   ENBL_IPV4_ONLY=0,     /**< Wan Connection is IPv4 only  */
   ENBL_IPV4_IPV6=1,     /**< Wan Connection is dual stack */
   ENBL_IPV6_ONLY=2      /**< Wan Connection is IPv6 only  */
}WanConnL3Type;


/* RAM size defines */
#define SZ_8MB          0x00800000
#define SZ_16MB         0x01000000
#define SZ_32MB         0x02000000
#define SZ_64MB         0x04000000
#define SZ_128MB        0x08000000
#define SZ_256MB        0x10000000


#define MAX_PERSISTENT_WAN_PORT     39       /**< Max Ethernet ports can be configured as WAN ports */
#define MAX_GMAC_ETH_PORT           5        /**< Max GMAC Ethernet ports in the system */

#define DNSINFO_CONF  "/var/dnsinfo.conf"   /**< This file is created by cms when there is a WAN connection status change and is used
                                               * by dnsproxy and can be used by other applications for the finding the WAN dns info.
                                               * and has the following fields,  WAN Interface; Subnet/Mask; dns1, dns2..; app1, app2.. :
                                               * eg. atm1;172.0.0.1/32;10.7.2.8,20.1.2.5;tr69c    -- WAN service for TR69
                                               * 1). WAN interface name. eg. ppp0, atm0, etc.
                                               * 2)  subnet/mask in cidr format (bind to this WAN)
                                               * 3) dns list for this WAN
                                               * 4) process name list uses this WAN
                                               */

/* include cms_params.h after we have defined all other constants. */
#include "cms_params.h"

/* contains either "gpon" or "epon" */
#define SUPPORTED_OPTICAL_WAN_TYPES "/proc/nvram/supported_optical_wan_types"

#ifdef DMP_DEVICE2_HOSTS_2
#ifdef DMP_X_BROADCOM_COM_DEV2_IPV6_1
#define HOST6_ACTIVES_FILE "/tmp/host6_actives"
#endif
#endif


typedef enum
{
   LOGIN_INVALID=0,     /**< This is for httpd login */
   LOGIN_USER=1,        /**< user login */
   LOGIN_SUPPORT=2,     /**< support login  */
   LOGIN_ADMIN=10,      /**< admin login  */
} HttpLoginType;

#endif /* __CMS_H__ */
