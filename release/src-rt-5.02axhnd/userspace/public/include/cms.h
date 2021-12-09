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

#include "os_defs.h"
#include "cms_version.h"



/*!\enum CmsRet
 * \brief Return codes for all external functions, and some internal functions too.
 *
 * Codes from 9000-9799 are reserved for TR69C return values.
 * All Broadcom return codes should start at 9800.
 */
typedef enum
{
   CMSRET_SUCCESS              = 0,     /**<Success. */
   CMSRET_METHOD_NOT_SUPPORTED = 9000,  /**<Method not supported. */
   CMSRET_REQUEST_DENIED       = 9001,  /**< Request denied (no reason specified). */
   CMSRET_INTERNAL_ERROR       = 9002,  /**< Internal error. */
   CMSRET_INVALID_ARGUMENTS    = 9003,  /**< Invalid arguments. */
   CMSRET_RESOURCE_EXCEEDED    = 9004,  /**< Resource exceeded.
                                        *  (when used in association with
                                        *  setParameterValues, this MUST not be
                                        *  used to indicate parameters in error)
                                        */
   CMSRET_INVALID_PARAM_NAME   = 9005,  /**< Invalid parameter name.
                                        *  (associated with set/getParameterValues,
                                        *  getParameterNames,set/getParameterAtrributes)
                                        */
   CMSRET_INVALID_PARAM_TYPE   = 9006,  /**< Invalid parameter type.
                                        *  (associated with set/getParameterValues)
                                        */
   CMSRET_INVALID_PARAM_VALUE  = 9007,  /**< Invalid parameter value.
                                        *  (associated with set/getParameterValues)
                                        */
   CMSRET_SET_NON_WRITABLE_PARAM = 9008,/**< Attempt to set a non-writable parameter.
                                        *  (associated with setParameterValues)
                                        */
   CMSRET_NOTIFICATION_REQ_REJECTED = 9009, /**< Notification request rejected.
                                            *  (associated with setParameterAttributes)
                                            */
   CMSRET_DOWNLOAD_FAILURE     = 9010,  /**< Download failure.
                                         *  (associated with download or transferComplete)
                                         */
   CMSRET_UPLOAD_FAILURE       = 9011,  /**< Upload failure.
                                        *  (associated with upload or transferComplete)
                                        */
   CMSRET_FILE_TRANSFER_AUTH_FAILURE = 9012,  /**< File transfer server authentication
                                              *  failure.
                                              *  (associated with upload, download
                                              *  or transferComplete)
                                              */
   CMSRET_UNSUPPORTED_FILE_TRANSFER_PROTOCOL = 9013,/**< Unsupported protocol for file
                                                    *  transfer.
                                                    *  (associated with upload or
                                                    *  download)
                                                    */
   CMSRET_FILE_TRANSFER_UNABLE_JOIN_MULTICAST = 9014,/**< File transfer failure,
                                                    *  unable to join multicast
                                                    *  group.
                                                    */
   CMSRET_FILE_TRANSFER_UNABLE_CONTACT_FILE_SERVER = 9015,/**< File transfer failure,
                                                    *  unable to contact file server.
                                                    */
   CMSRET_FILE_TRANSFER_UNABLE_ACCESS_FILE = 9016,/**< File transfer failure,
                                                    *  unable to access file.
                                                    */
   CMSRET_FILE_TRANSFER_UNABLE_COMPLETE = 9017,/**< File transfer failure,
                                                    *  unable to complete download.
                                                    */
   CMSRET_FILE_TRANSFER_FILE_CORRUPTED = 9018,/**< File transfer failure,
                                                    *  file corrupted.
                                                    */
   CMSRET_FILE_TRANSFER_FILE_AUTHENTICATION_ERROR = 9019,/**< File transfer failure,
                                                    *  file authentication error.
                                                    */
   CMSRET_FILE_TRANSFER_FILE_TIMEOUT = 9020,/**< File transfer failure,
                                                    *  download timeout.
                                                    */
   CMSRET_FILE_TRANSFER_FILE_CANCELLATION_NOT_ALLOW = 9021,/**< File transfer failure,
                                                    *  cancellation not permitted.
                                                    */
   CMSRET_INVALID_UUID_FORMAT = 9022,/**< Invalid UUID Format
                                                    * (associated with ChangeDUState)
                                                    */
   CMSRET_UNKNOWN_EE = 9023,/**< Unknown Execution Environment
                                                    * (associated with ChangeDUState)
                                                    */

   CMSRET_EE_DISABLED = 9024,/**< Execution Environment disabled
                                                    * (associated with ChangeDUState)
                                                    */
   CMSRET_DU_EE_MISMATCH = 9025,/**< Execution Environment and Deployment Unit mismatch
                                                    * (associated with ChangeDUState:install/update)
                                                    */
   CMSRET_DU_DUPLICATE = 9026,/**< Duplicate Deployment Unit
                                                    * (associated with ChangeDUState:install/update)
                                                    */
   CMSRET_SW_MODULE_SYSTEM_RESOURCE_EXCEEDED = 9027,/**< System resources exceeded
                                                    * (associated with ChangeDUState:install/update)
                                                    */
   CMSRET_DU_UNKNOWN = 9028,/**< Unknown Deployment Unit
                                                    * (associated with ChangeDUState:update/uninstall)
                                                    */
   CMSRET_DU_STATE_INVALID = 9029,/**< Invalid Deployment Unit State
                                                    * (associated with ChangeDUState:update)
                                                    */
   CMSRET_DU_UPDATE_DOWNGRADE_NOT_ALLOWED = 9030,/**< Invalid Deployment Unit Update, downgrade not permitted
                                                    * (associated with ChangeDUState:update)
                                                    */
   CMSRET_DU_UPDATE_VERSION_NOT_SPECIFIED = 9031,/**< Invalid Deployment Unit Update, version not specified
                                                    * (associated with ChangeDUState:update)
                                                    */

   CMSRET_DU_UPDATE_VERSION_EXISTED= 9032,/**< Invalid Deployment Unit Update, version already exists
                                                    * (associated with ChangeDUState:update)
                                                    */
                                                    
   CMSRET_SUCCESS_REBOOT_REQUIRED = 9800, /**< Config successful, but requires reboot to take effect. */
   CMSRET_SUCCESS_UNRECOGNIZED_DATA_IGNORED = 9801,  /**<Success, but some unrecognized data was ignored. */
   CMSRET_SUCCESS_OBJECT_UNCHANGED = 9802,  /**<Success, furthermore object has not changed, returned by STL handler functions. */
   CMSRET_SUCCESS_APPLY_NOT_COMPLETE = 9803, /**< Config validated/commited, but requires more action to take effect. */
   CMSRET_NO_MORE_INSTANCES = 9804,     /**<getnext operation cannot find any more instances to return. */
   CMSRET_MDM_TREE_ERROR = 9805,         /**<Error during MDM tree traversal */
   CMSRET_WOULD_DEADLOCK = 9806, /**< Caller is requesting a lock while holding the same lock or a different one. */
   CMSRET_LOCK_REQUIRED = 9807,  /**< The MDM lock is required for this operation. */
   CMSRET_OP_INTR = 9808,      /**<Operation was interrupted, most likely by a Linux signal. */
   CMSRET_TIMED_OUT = 9809,     /**<Operation timed out. */
   CMSRET_DISCONNECTED = 9810,  /**< Communications link is disconnected. */
   CMSRET_MSG_BOUNCED = 9811,   /**< Msg was sent to a process not running, and the
                                 *   bounceIfNotRunning flag was set on the header.  */
   CMSRET_OP_ABORTED_BY_USER = 9812,  /**< Operation was aborted/discontinued by the user */
   CMSRET_FAIL_REBOOT_REQUIRED = 9813,  /**<Config failed, and now system is in a bad state requiring reboot. */
   CMSRET_ACCESS_DENIED = 9814,  /**< Data model access denied (no reason specified). */
   CMSRET_OPERATION_NOT_PERMITTED= 9815,  /**< Operation not permitted (errno EPERM) */
   CMSRET_RECURSION_ERROR = 9817,     /**< too many levels of recursion */
   CMSRET_OPEN_FILE_ERROR = 9818,     /**< open file error */
   CMSRET_EAGAIN_ERROR = 9820,        /**< socket write EAGAIN error */
   CMSRET_SOCKET_ERROR = 9821,        /**< socket error */
   CMSRET_KEY_GENERATION_ERROR = 9830,     /** certificate key generation error */
   CMSRET_INVALID_CERT_REQ = 9831,     /** requested certificate does not match with issued certificate */
   CMSRET_INVALID_CERT_SUBJECT = 9832,     /** certificate has invalid subject information */
   CMSRET_OBJECT_NOT_FOUND = 9840,     /** failed to find object */

   CMSRET_INVALID_FILENAME = 9850,  /**< filename was not given for download */
   CMSRET_INVALID_IMAGE = 9851,     /**< bad image was given for download */
   CMSRET_INVALID_CONFIG_FILE = 9852,  /**< invalid config file was detected */
   CMSRET_CONFIG_PSI = 9853,         /**< old PSI/3.x config file was detected */
   CMSRET_IMAGE_FLASH_FAILED = 9854, /**< could not write the image to flash */
   CMSRET_RESOURCE_NOT_CONFIGURED = 9855, /**< requested resource is not configured/found */
   CMSRET_EE_UNPACK_ERROR = 9856,   /**< EE download unpack failure (associated with ChangeEEState) */
   CMSRET_EE_DUPLICATE = 9857,   /**< Duplicate Execution Environment (associated with ChangeEEState:install) */
   CMSRET_EE_UPDATE_DOWNGRADE_NOT_ALLOWED = 9858,/**< Invalid Execution Environment Update, downgrade not permitted
                                                    * (associated with ChangeEEState:update)
                                                    */
   CMSRET_EE_UPDATE_VERSION_NOT_SPECIFIED = 9859,/**< Invalid Execution Environment Update, version not specified
                                                    * (associated with ChangeEEState:update)
                                                    */
   CMSRET_EE_UPDATE_VERSION_EXISTED = 9860,/**< Invalid Execution Environment Update, version already exists
                                                    * (associated with ChangeEEState:update)
                                                    */
   CMSRET_PMD_CALIBRATION_FILE_SUCCESS = 9861, /**<Success. PMD Calibration file was uploaded.*/
   CMSRET_PMD_TEMP2APD_FILE_SUCCESS = 9862, /**<Success. PMD Temp2Apd file was uploaded.*/
   CMSRET_MANIFEST_PARSE_ERROR = 9863,   /**< Manifest parse error. */
   CMSRET_USERNAME_IN_USE = 9864,   /**< Username in use. */
   CMSRET_ADD_USER_ERROR = 9865,    /**< Add user error. */
   CMSRET_DELETE_USER_ERROR = 9866, /**< Delete user error. */
   CMSRET_USER_NOT_FOUND = 9867,    /**<User not found. */
   CMSRET_SET_BUSGATE_POLICY_ERROR = 9868,   /**<Set busgate policy error. */
   CMSRET_OPERATION_IN_PROCESS = 9869,    /**<Operation is still in process. */
} CmsRet;


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
#ifdef SUPPORT_CELLULAR
   WAN_IFC_CELLULAR=6,  /**< Cellular */
#endif   
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
#endif

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
   ATM=0,          /**< WanDev is used for DSL ATM  */               
   PTM=1,          /**< WanDev is used for DSL PTM  */     
   Ethernet=2      /**< WanDev is used for Ethernet  */
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


typedef enum
{
   LOGIN_INVALID=0,     /**< This is for httpd login */
   LOGIN_USER=1,        /**< user login */     
   LOGIN_SUPPORT=2,     /**< support login  */
   LOGIN_ADMIN=10,      /**< admin login  */       
} HttpLoginType;

#endif /* __CMS_H__ */
