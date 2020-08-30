/*
* <:copyright-BRCM:2014:proprietary:standard
*
*    Copyright (c) 2014 Broadcom
*    All Rights Reserved
*
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
*
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
*
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
:>
*/

#ifndef __CMS_DAL_H__
#define __CMS_DAL_H__


/*!\file cms_dal.h
 * \brief Header file for the Data Aggregation Layer API.
 * This API is used by httpd, cli, and snmp and presents an API similar to the
 * old dbAPI.  A lot of code in http, cli, and snmp rely on functions that are
 * more in the old dbAPI style than the new object layer API.
 *
 * This API is implemented in the cms_dal.so library.
 */

#include "cms_core.h"
#include "cms_msg.h"  /* for NetworkAccessMode enum */
#include "beep_networking.h"  /* for IntfGrpBridgeMode enum */

/** Board data buf len (legacy) ?
 */
#define WEB_MD_BUF_SIZE_MAX   264

/** Length of TR69C_URL */
#define TR69C_URL_LEN         258

/** PortMapping Interface list size */
#define PMAP_INTF_LIST_SIZE 724

/** Legacy structure used by httpd and cli to hold a bunch of variables.
 */
typedef struct {
   char adminUserName[BUFLEN_16];
   char adminPassword[BUFLEN_48];
   char sptUserName[BUFLEN_16];
   char sptPassword[BUFLEN_48];
   char usrUserName[BUFLEN_16];
   char usrPassword[BUFLEN_48];
   char curUserName[BUFLEN_16];
   char inUserName[BUFLEN_16];
   char inPassword[BUFLEN_48];
   char inOrgPassword[BUFLEN_48];
   SINT32 recvSessionKey;  /**< Session key received from browser, used by httpd only */
   UBOOL8 ipAddrAccessControlEnabled; /**< For the IP address access control list mechanism */
   char brName[BUFLEN_64];
   char ethIpAddress[BUFLEN_16];
   char ethSubnetMask[BUFLEN_16];
   UBOOL8 enblLanFirewall;          /**< Is LAN side firewall enabled */
   char lan2IpAddress[BUFLEN_16];
   char lan2SubnetMask[BUFLEN_16];
   SINT32 enblLan2;
   SINT32 dataModelDevice2;         /**< Is Data Model PURE TR-181 */
   char wanL2IfName[BUFLEN_32];     /**< The layer 2 interface name of a service connection */
   char wanIpAddress[BUFLEN_16];
   char wanSubnetMask[BUFLEN_16];
   char defaultGatewayList[CMS_MAX_DEFAULT_GATEWAY * CMS_IFNAME_LENGTH];  /**< max default gateways allowed in the system */
   char wanIntfGateway[BUFLEN_16];  /**< For static IPoE WAN gateway ip (could served as system default gateway if it is seleected) */
   char wanIfName[BUFLEN_32];
   char dnsIfcsList[CMS_MAX_DNSIFNAME * CMS_IFNAME_LENGTH];      /**< max wan interfaces allowed in dns interface list */
   char dnsPrimary[CMS_IPADDR_LENGTH];
   char dnsSecondary[CMS_IPADDR_LENGTH];
   char dnsHostName[BUFLEN_32];   /**< dnsHostName (was IFC_SMALL_LEN) */
   char dnsDomainName[BUFLEN_32]; /**< dnsDomainName */
   char dhcpEthStart[BUFLEN_16];
   char dhcpEthEnd[BUFLEN_16];
   char dhcpSubnetMask[BUFLEN_16]; /**< added by lgd for TR98, IFC_TINY_LEN */
   SINT32  dhcpLeasedTime;     /**< this is in hours, MDM stores it in seconds */
   char dhcpRelayServer[BUFLEN_16];
   char pppUserName[BUFLEN_256];
   char pppPassword[BUFLEN_40];
   char pppServerName[BUFLEN_40];
   char serviceName[BUFLEN_256+1];
   SINT32  serviceId;
#if defined(DMP_X_BROADCOM_COM_GPONWAN_1)
   SINT32  noMcastVlanFilter; /**< If multicast enabled on WAN - does it need VLAN filtering */
#endif /* DMP_X_BROADCOM_COM_GPONWAN_1 */
#ifdef SUPPORT_CELLULAR
   UBOOL8  cellularRoamingEnbl;
   UBOOL8  cellularIntfEnable;
   char cellularSupportedAccTech[BUFLEN_64];
   char cellularPrefdAccTech[BUFLEN_16];
   char cellularNtwkReq[BUFLEN_64];
   char cellularNtwkReq_MCCMNC[BUFLEN_8];
   char cellularPinChk[BUFLEN_64];
   char cellularPin[BUFLEN_8];
#endif
   SINT32  enblService;
   SINT32  ntwkPrtcl;
   SINT32  encapMode;
   SINT32  enblDhcpClnt;
   SINT32  enblDhcpSrv;
   SINT32  enblAutoScan;
   SINT32  enblNat;
   SINT32  enblFullcone;  /**< enable fullcone nat */
   SINT32  enblFirewall;
   SINT32  enblOnDemand;
   SINT32  pppTimeOut;
   SINT32  pppIpExtension;
   SINT32  pppAuthMethod;
   SINT32  pppShowAuthErrorRetry;
   SINT32  pppAuthErrorRetry;
   SINT32  enblPppDebug;
   SINT32  pppToBridge;  /**< ppp to bridge */

   SINT32  logStatus;   /**< Enabled or disabled */
   SINT32  logDisplay;  /**< What level to display when asked to dump log */
   SINT32  logLevel;    /**< What level to log at */
   SINT32  logMode;     /**< 1=local, 2=remote, 3=local and remote */
   char logIpAddress[BUFLEN_16];  /* Remote log server IP address */
   SINT32  logPort;     /**< Remote log server port */

   UINT32  adslFlag;  /**< various bits in the WanDslInterfaceConfig object. */
#ifdef SUPPORT_RIP
   SINT32  ripMode;
   SINT32  ripVersion;
   SINT32  ripOperation;
#endif

#ifdef SUPPORT_DSL
   char dslPhyDrvVersion[BUFLEN_64];  /**< dsl phy and driver version */
#endif

#ifdef SUPPORT_LANVLAN
   char    lanName[BUFLEN_32];
   UINT32  lanVlanEnable;
   char    lanVlanTagList[BUFLEN_128];
#endif
   SINT32  portId;   /**< port Id, dual latency stuff */
   SINT32  ptmPriorityNorm; /**< PTM mode priority,  default priorityNorm is TRUE */
   SINT32  ptmPriorityHigh; /**< PTM mode priority,  default priorityHigh is FALSE */
   SINT32  atmVpi;
   SINT32  atmVci;
   SINT32  connMode;  /**< connection mode for Default=0, VLAN=1 and MSC=3 */
   SINT32  enVlanMux;  /**< enable VLAN mux */
   SINT32  vlanMuxPr;  /**< VLAN 8021p priority */
   SINT32  vlanMuxId;  /**< VLAN mux ID */
   UINT32  vlanTpid;  /**< VLAN TPID */
   SINT32  atmPeakCellRate;      /**< really should be UINT32 to match data-model */
   SINT32  atmSustainedCellRate; /**< really should be UINT32 to match data-model */
   SINT32  atmMaxBurstSize;      /**< really should be UINT32 to match data-model */
   SINT32  atmMinCellRate;
   char atmServiceCategory[BUFLEN_16];
   char linkType[BUFLEN_16];  /**< Link type, EoA, IPOA, PPPOA */
   char grpScheduler[BUFLEN_8]; /**< WRR */
   UINT32  grpWeight;
   UINT32  grpPrecedence;
   char schedulerAlgorithm[BUFLEN_8];  /**< SP, WRR, WFQ */
   UINT32  queueWeight;
   UINT32  queuePrecedence;
   char dropAlgorithm[BUFLEN_8];
   UINT32  loMinThreshold;
   UINT32  loMaxThreshold;
   UINT32  hiMinThreshold;
   UINT32  hiMaxThreshold;
   SINT32  queueMinimumRate;
   SINT32  queueShapingRate;
   UINT32  queueShapingBurstSize;
   SINT32  snmpStatus;
   char snmpRoCommunity[BUFLEN_16];
   char snmpRwCommunity[BUFLEN_16];
   char snmpSysName[BUFLEN_16];
   char snmpSysLocation[BUFLEN_16];
   char snmpSysContact[BUFLEN_16];
   char snmpTrapIp[BUFLEN_16];
   SINT32  macPolicy;
   SINT32  enblQos;
   SINT32  enblDiffServ;

   SINT32  qosClsKey;
   char qosClsName[BUFLEN_16];
   SINT32  qosClsOrder;
   SINT32  qosClsEnable;
   SINT32  qosProtocol;
   SINT32  qosWanVlan8021p;
   SINT32  ipoptionlist;
   char optionval[BUFLEN_16];
   SINT32  qosVlan8021p;
   char qosSrcAddr[BUFLEN_16];
   char qosSrcMask[BUFLEN_16];
   char qosSrcPort[BUFLEN_16];
   char qosDstAddr[BUFLEN_16];
   char qosDstMask[BUFLEN_16];
   char qosDstPort[BUFLEN_16];
   SINT32  qosDscpMark;
   SINT32  qosDscpCheck;
   char qosSrcMacAddr[BUFLEN_32];
   char qosDstMacAddr[BUFLEN_32];
   char qosSrcMacMask[BUFLEN_32];
   char qosDstMacMask[BUFLEN_32];
   SINT32  qosClsQueueKey;
   SINT32  qosProtocolExclude;
   SINT32  qosSrcPortExclude;
   SINT32  qosSrcIPExclude;
   SINT32  qosDstIPExclude;
   SINT32  qosDstPortExclude;
   SINT32  qosDscpChkExclude;
   SINT32  qosEtherPrioExclude;
   char qosLanIfcName[BUFLEN_16];

#ifdef SUPPORT_QUICKSETUP
   SINT32  quicksetupErrCode;
#endif

#ifdef SUPPORT_SAMBA
   char storageuserName[BUFLEN_64];
   char storagePassword[BUFLEN_64];
   char storagevolumeName[BUFLEN_64];
#endif

#ifdef SUPPORT_UPNP
   SINT32  enblUpnp;
#endif
#ifdef ETH_CFG
   SINT32  ethSpeed;
   SINT32  ethType;
   SINT32  ethMtu;
#endif
#if SUPPORT_PORT_MAP
   char groupName[BUFLEN_16];  /**< name of port group, also user friendly name of bridge */
   SINT32  editNtwkPrtcl;
   SINT32  editPortId ;
   SINT32  editAtmVpi;
   SINT32  editAtmVci;
#endif

   /*
    * Ethernet switch configuration is related to port mapping, but is not only for port mapping,
    * so it is outside of the SUPPORT_PORT_MAP define.
    */
   UBOOL8  virtualPortsEnabled;   /**< Is virtual ports enabled on the ethernet switch */
   UINT32  numberOfVirtualPorts;  /**< A value of 2 or more indicates ethernet switch supports virtual ports */
   char    ethSwitchIfName[BUFLEN_32];  /**< Base name of the ethernet switch */

   /* define this even if SUPPORT_ETHWAN is not defined to make code cleaner */
   SINT32 enableEthWan;

#ifdef SUPPORT_ADVANCED_DMZ
   SINT32  enableAdvancedDmz;  /**< Added by Keven */
   char    nonDmzIpAddress[BUFLEN_16];
   char    nonDmzIpMask[BUFLEN_16];
#endif
   SINT32  useStaticIpAddress;
   char pppLocalIpAddress[BUFLEN_16];
#ifdef SUPPORT_VLAN
   SINT32 enblVlan;
   SINT32 vlanId;
#endif
#ifdef SUPPORT_IPSEC
   SINT32 ipsTableIndex;
   SINT32 enable;
   char ipsConnName[BUFLEN_40];
   char ipsTunMode[BUFLEN_16];
   char ipsIpver[BUFLEN_16];
   char ipsLocalGwIf[BUFLEN_32];
   char ipsRemoteGWAddr[CMS_IPADDR_LENGTH];
   char ipsLocalIPMode[BUFLEN_16];
   char ipsLocalIP[CMS_IPADDR_LENGTH];
   char ipsLocalMask[BUFLEN_40];
   char ipsRemoteIPMode[BUFLEN_16];
   char ipsRemoteIP[CMS_IPADDR_LENGTH];
   char ipsRemoteMask[BUFLEN_40];
   char ipsKeyExM[BUFLEN_16];
   char ipsAuthM[BUFLEN_16];
   char ipsPSK[BUFLEN_16];
   char ipsCertificateName[BUFLEN_40];
   SINT32 ipsKeyTime;
   char ipsPerfectFSEn[BUFLEN_16];
   char ipsManualEncryptionAlgo[BUFLEN_16];
   char ipsManualEncryptionKey[BUFLEN_256];
   char ipsManualAuthAlgo[BUFLEN_16];
   char ipsManualAuthKey[BUFLEN_256];
   char ipsSPI[BUFLEN_40];
   char ipsPh1Mode[BUFLEN_16];
   char ipsPh1EncryptionAlgo[BUFLEN_16];
   char ipsPh1IntegrityAlgo[BUFLEN_16];
   char ipsPh1DHGroup[BUFLEN_16];
    SINT32 ipsPh1KeyTime;
   char ipsPh2EncryptionAlgo[BUFLEN_16];
   char ipsPh2IntegrityAlgo[BUFLEN_16];
   char ipsPh2DHGroup[BUFLEN_16];
   SINT32 ipsPh2KeyTime;
#endif
#ifdef SUPPORT_CERT
   char certCategory[BUFLEN_16];
   char certName[BUFLEN_16];
#endif
#ifdef PORT_MIRRORING
   SINT32  mirrorPort ;
   SINT32  mirrorStatus ;
#endif
#ifdef SUPPORT_TR69C
   UBOOL8  tr69cInformEnable;   /**< InformEnabled */
   SINT32  tr69cInformInterval; /**< Inform interval, in seconds? */
   UBOOL8  tr69cNoneConnReqAuth; /**< NoneConnReqAuth */
   UBOOL8  tr69cDebugEnable;   /**< DebugEnabled */
   char tr69cAcsURL[TR69C_URL_LEN];  /**< URL of ACS server */
   char tr69cAcsUser[TR69C_URL_LEN]; /**< username for ACS server */
   char tr69cAcsPwd[TR69C_URL_LEN];  /**< password for ACS server */
   char tr69cConnReqURL[TR69C_URL_LEN]; /**< Inbound connection request URL */
   char tr69cConnReqUser[TR69C_URL_LEN]; /**< Inbound connection request username */
   char tr69cConnReqPwd[TR69C_URL_LEN];  /**< Inbound connection request password */
   char tr69cBoundIfName[BUFLEN_32];     /**< Interface which tr69c uses to talk to ACS */
#endif
#ifdef DMP_X_ITU_ORG_GPON_1 /**< aka SUPPORT_OMCI */
   UINT32 omciTcontNum;
   UINT32 omciTcontMeId;
   UINT32 omciEthNum;
   UINT32 omciEthMeId1;
   UINT32 omciEthMeId2;
   UINT32 omciEthMeId3;
   UINT32 omciEthMeId4;
   UINT32 omciEthMeId5;
   UINT32 omciEthMeId6;
   UINT32 omciEthMeId7;
   UINT32 omciEthMeId8;
   UINT32 omciMocaNum;
   UINT32 omciMocaMeId1;
   UINT32 omciDsPrioQueueNum;
   UBOOL8 omciDbgOmciEnable;
   UBOOL8 omciDbgModelEnable;
   UBOOL8 omciDbgVlanEnable;
   UBOOL8 omciDbgCmfEnable;
   UBOOL8 omciDbgFlowEnable;
   UBOOL8 omciDbgRuleEnable;
   UBOOL8 omciDbgMcastEnable;
   UBOOL8 omciDbgFileEnable;
   UINT32 omciRawEnable;
#endif
   char swVers[WEB_MD_BUF_SIZE_MAX];  /**< Software Version */
   char boardID[BUFLEN_16];   /**< Board Id */
   char cfeVers[BUFLEN_32];   /**< cfe version string */
   char cmsBuildTimestamp[BUFLEN_32];   /**< CMS build timestamp */
   char pcIpAddr[CMS_IPADDR_LENGTH];
   UINT32 numCpuThreads;      /**< Number of CPU threads */

   UINT32 enblIpVer;
   UINT32 pcpMode;
   char   pcpServer[CMS_IPADDR_LENGTH];
#ifdef SUPPORT_IPV6
   /* enblDhcp6s and enblRadvd are added for easy seperation on igd and dev2 */
   UBOOL8 enblDhcp6s;
   UBOOL8 enblRadvd;

   UBOOL8 unnumberedModel;
   UBOOL8 enblDhcp6sStateful;
   char ipv6IntfIDStart[CMS_IPADDR_LENGTH];
   char ipv6IntfIDEnd[CMS_IPADDR_LENGTH];
   SINT32 dhcp6LeasedTime; /**< this is in hours, MDM stores it in seconds */
   char dns6Type[BUFLEN_16];
   char dns6Ifc[BUFLEN_32];
   char dns6Pri[CMS_IPADDR_LENGTH];
   char dns6Sec[CMS_IPADDR_LENGTH];
   UBOOL8 enblRadvdUla;
   UBOOL8 enblRandomULA;
   char ipv6UlaPrefix[CMS_IPADDR_LENGTH];
   SINT32 ipv6UlaPlt;
   SINT32 ipv6UlaVlt;
   char wanAddr6Type[BUFLEN_16];
   UBOOL8 dhcp6cForAddr;
   UBOOL8 dhcp6cForPd;
   char wanAddr6[CMS_IPADDR_LENGTH];
   char wanGtwy6[CMS_IPADDR_LENGTH];
   char dfltGw6Ifc[BUFLEN_32];
   char lanIntfAddr6[CMS_IPADDR_LENGTH];
#endif
#if defined(DMP_TIME_1) || defined(DMP_DEVICE2_TIME_1)
   UBOOL8 NTPEnable;/**< Enable NTP server */
   char NTPServer1[BUFLEN_64];	/**< NTPServer1 */
   char NTPServer2[BUFLEN_64];	/**< NTPServer2 */
   char NTPServer3[BUFLEN_64];	/**< NTPServer3 */
   char NTPServer4[BUFLEN_64];	/**< NTPServer4 */
   char NTPServer5[BUFLEN_64];	/**< NTPServer5 */
   char localTimeZone[BUFLEN_8];	/**< LocalTimeZone */
   char localTimeZoneName[BUFLEN_64];	/**< LocalTimeZoneName */
   UBOOL8 daylightSavingsUsed;	/**< DaylightSavingsUsed */
   char  daylightSavingsStart[BUFLEN_64];	/**< DaylightSavingsStart */
   char  daylightSavingsEnd[BUFLEN_64];	/**< DaylightSavingsEnd */
#endif
   char vdslSoftwareVersion[BUFLEN_64];  /**< VDSL software version */
   char dhcpcOp60VID[BUFLEN_256];
   char dhcpcOp61DUID[BUFLEN_256];
   char dhcpcOp61IAID[BUFLEN_16];
   char dhcpcOp77UID[BUFLEN_256];
   UBOOL8 dhcpcOp125Enabled;
   UINT32 dhcpcOp51LeasedTime;
   char dhcpcOp50IpAddress[BUFLEN_16];
   char dhcpcOp54ServerIpAddress[BUFLEN_16];
#if defined (DMP_X_BROADCOM_COM_PWRMNGT_1) /* aka SUPPORT_PWRMNGT */
   UINT32 pmCPUSpeed;         /* CPU speed 1,2, 4 and 8 are valid */
   UBOOL8 pmCPUr4kWaitEn;       /* r4k wait state sleep to enable */
   UBOOL8 pmDRAMSelfRefreshEn;  /* DRAM Self-Refresh mode */
   UBOOL8 pmEthAutoPwrDwnEn;    /* Ethernet Auto Power Down */
   UBOOL8 pmEthEEE; /* Energy Efficient Ethernet Enable */
   UBOOL8 pmAvsEn;               /* Adaptive Voltage Scaling */
#endif /* aka SUPPORT_PWRMNGT */
#ifdef SUPPORT_BMU
   char bmuVersion[BUFLEN_64];
   char bmuBuildDateTime[BUFLEN_32];
   char bmuPowerSource[BUFLEN_16];
   char bmuState[BUFLEN_32];
   UINT32 bmuNumberOfPresentBatteries;
   char bmuInputVoltage[BUFLEN_16];
   UINT32 bmuTemperature;
   UINT32 bmuEstimatedMinutesRemaining;
   UINT32 bmuUpsSecondsOnBattery;
   char bmuStatusBatteryA[BUFLEN_32];
   char bmuStatusBatteryB[BUFLEN_32];
   UINT32 bmuCapacityBatteryA;
   UINT32 bmuCapacityBatteryB;
   char bmuMeasuredVoltageBatteryA[BUFLEN_16];
   char bmuMeasuredVoltageBatteryB[BUFLEN_16];
   UINT32 bmuEstimatedTimeRemainingBatteryA;
   UINT32 bmuEstimatedTimeRemainingBatteryB;
#endif
#ifdef DMP_X_BROADCOM_COM_STANDBY_1
   UBOOL8 pmStandbyEnable;                 /* To enable Standby feature */
   char pmStandbyStatusString[BUFLEN_64];  /* To report the current status of the standby functionality */
   UINT32 pmStandbyHour;                   /* Time to go in Standby */
   UINT32 pmStandbyMinutes;                /* Time to go in Standby */
   UINT32 pmWakeupHour;                    /* Time to wake up */
   UINT32 pmWakeupMinutes;                 /* Time to wake up */
#endif
#ifdef SUPPORT_DSL_BONDING
   int dslBonding;
   int dslBondingStatus;
#endif
   UINT32 bondingLineNum;  /**< DSL line number, used even in non-bonding builds */
#ifdef DMP_X_BROADCOM_COM_MCAST_1
   int    mcastPrecedence;	/**< mcastPrecedence */
   int    mcastStrictWan;	 /**< mcastStrictWan */
   char   igmpExceptAddressList[BUFLEN_1024];
   char   mldExceptAddressList[BUFLEN_1024];
#ifdef DMP_X_BROADCOM_COM_DCSP_MCAST_REMARK_1
   UBOOL8 mcastDscpRemarkEnable;
   int    mcastDscpRemarkVal;
#endif
#endif
   UBOOL8 igmpSnpSup;   /* indicates whether or not IGMP snooping is supported */
#ifdef DMP_X_BROADCOM_COM_IGMP_1
   SINT32 enblIgmp;             /* indicates whether or not IGMP proxy is enabled on this interface */
   SINT32 enblIgmpMcastSource;  /* Allow use of this interface as an IGMP multicast source, even in absence of proxy */
   UINT32 igmpVer;	/**< IgmpVer */
   UINT32 igmpQI;	/**< IgmpQI */
   UINT32 igmpQRI;	/**< IgmpQRI */
   UINT32 igmpLMQI;	/**< IgmpLMQI */
   UINT32 igmpRV;	/**< IgmpRV */
   UINT32 igmpMaxGroups;	/**< IgmpMaxGroups */
   UINT32 igmpMaxSources;	/**< IgmpMaxSources */
   UINT32 igmpMaxMembers;	/**< IgmpMaxMembers */
   UBOOL8 igmpFastLeaveEnable;	/**< IgmpFastLeaveEnable */
#endif /* SUPPORT_IGMP */
#if defined(DMP_X_BROADCOM_COM_IGMPSNOOP_1)
   SINT32  enblIgmpSnp;  /**< igmp snooping (per lan interface), configured into bridge */
   SINT32  enblIgmpMode; /**< 0 means standard, 1 means blocking */
   SINT32  enblIgmpLanToLanMcast;
#endif
   UBOOL8 mldSnpSup;   /* indicates whether or not MLD snooping is supported */
#ifdef DMP_X_BROADCOM_COM_MLD_1
   SINT32 enblMld;             /* indicates whether or not MLD proxy is enabled on this interface */
   SINT32 enblMldMcastSource;  /* Allow use of this interface as an MLD multicast source, even in absence of proxy */
   UINT32 mldVer;	/**< MldVer */
   UINT32 mldQI;	/**< MldQI */
   UINT32 mldQRI;	/**< MldQRI */
   UINT32 mldLMQI;	/**< MldLMQI */
   UINT32 mldRV;	/**< MldRV */
   UINT32 mldMaxGroups;	/**< MldMaxGroups */
   UINT32 mldMaxSources;	/**< MldMaxSources */
   UINT32 mldMaxMembers;	/**< MldMaxMembers */
   UBOOL8 mldFastLeaveEnable;	/**< MldFastLeaveEnable */
#endif
#if defined(DMP_X_BROADCOM_COM_MLDSNOOP_1)
   UBOOL8 enblMldSnp;  /**< MLD snooping (should be per lan interface), configured into bridge */
   SINT32 enblMldMode; /**< 0 means standard, 1 means blocking */
   SINT32 enblMldLanToLanMcast;
#endif
   SINT32 cfgL2tpAc;                      /**< If set to 1, in configuring PPPoL2tpAC mode */
#ifdef DMP_X_BROADCOM_COM_L2TPAC_1
   char lnsIpAddress[CMS_IPADDR_LENGTH];  /**< L2TP server ip address */
   char tunnelName[BUFLEN_64];            /**< tunnel name */
#endif /* DMP_X_BROADCOM_COM_L2TPAC_1 */
   SINT32 cfgPptpAc;                      /**< If set to 1, in configuring PPTP mode */
#ifdef DMP_X_BROADCOM_COM_PPTPAC_1
   char pnsIpAddress[CMS_IPADDR_LENGTH];  /**< PPTP server ip address */
   char pptpTunnelName[BUFLEN_64];            /**< tunnel name */
#endif /* DMP_X_BROADCOM_COM_PPTPAC_1 */

#ifdef SUPPORT_MOCA
   char mocaVersion[BUFLEN_128];  /**< Version string for moca */
   char mocaIfName[BUFLEN_16];    /**< Interface name for moca */
   SINT32 enblMocaPrivacy;        /**< MoCA Privacy enable setting */
   char mocaPassword[BUFLEN_18];  /**< MoCA Privacy password */
   SINT32 enblMocaAutoScan;       /**< MoCA Auto network search setting */
   SINT32 mocaFrequency;          /**< MoCA Last operating frequency */
#endif

#ifdef SUPPORT_HOMEPLUG
   char homeplugStatus[BUFLEN_16];           /**< Device status */
   UINT32 homeplugMaxBitRate;                /**< PHY Maximum bit rate */
   char homeplugMACAddress[BUFLEN_64];       /**< MAC Address */
   char plcVersion[BUFLEN_64];               /**< Firmware version string */
   char homeplugVersion[BUFLEN_64];          /**< HomePlug version string */
   char homeplugNetworkPassword[BUFLEN_32];  /**< Network password */
   char homeplugAlias[BUFLEN_64];            /**< Network alias */
#endif

#ifdef SUPPORT_IEEE1905
    SINT32 ieee1905IsEnabled;          /**< true if ieee1905 is enabled */
    char   ieee1905Dfname[BUFLEN_64];  /**< device friendly name */
    SINT32 ieee1905IsRegistrar;        /**< true if node is registrar */
    SINT32 ieee1905ShowApFreq24;       /**< true if there is an adaptor configured for 2.4 */
    SINT32 ieee1905ApFreq24En;         /**< true if 2.4Ghz support for AP auto configuration is enabled */
    SINT32 ieee1905ShowApFreq5;        /**< true if there is an adaptor configured for 5 */
    SINT32 ieee1905ApFreq5En;          /**< true if 5.0Ghz support for AP auto configuration is enabled */
#endif

#ifdef DMP_X_BROADCOM_COM_ETHERNETOAM_1
   UBOOL8 eoam3ahEnbl;
   char eoam3ahIntf[CMS_IFNAME_LENGTH];
   SINT32 eoam3ahOamId;
   UBOOL8 eoam3ahAeEnbl;
   UBOOL8 eoam3ahVrEnbl;
   UBOOL8 eoam3ahLeEnbl;
   UBOOL8 eoam3ahRlEnbl;
   UBOOL8 eoam3ahAmEnbl;
   UBOOL8 eoam1agEnbl;
   UBOOL8 eoam1731Enbl;
   char eoam1agMdId[BUFLEN_48];
   SINT32 eoam1agMdLvl;
   char eoam1agMaId[BUFLEN_48];
   SINT32 eoam1agCcmInterval;
   char eoam1agLocIntf[CMS_IFNAME_LENGTH];
   SINT32 eoam1agLocMepId;
   SINT32 eoam1agLocVlanId;
   UBOOL8 eoam1agLocCcmEnbl;
   SINT32 eoam1agRemMepId;
#endif

#ifdef DMP_X_BROADCOM_COM_PSTNENDPOINT_1
   char voiceServiceVersion[BUFLEN_64];  /**< Version string for our VOIP service */
#endif
#ifdef DMP_X_BROADCOM_COM_DBUSREMOTE_1
   UBOOL8 dbusRemoteEnable;
   UINT32 dbusPort;
#endif

#ifdef DMP_X_BROADCOM_COM_SPDSVC_1
   char speedTestMode[BUFLEN_16];
   char speedTestServerAddr[BUFLEN_32];
   char speedTestDirection[BUFLEN_16];
   char speedTestDataPath[BUFLEN_16];
   char speedTestAlgorithm[BUFLEN_8];
   UINT32 speedTestTcpPort;
   UINT32 speedTestDuration;
   UINT32 speedTestPktLength;
   UINT32 speedTestKbps;
   UINT32 speedTestSteps;
   UINT32 speedTestLoss;
   SINT32 speedTestLatency;
   UINT32 speedTestLossPercentage;
#endif
#ifdef DMP_X_BROADCOM_COM_EPON_1
   char oamAuthLoid[BUFLEN_32];
   char oamAuthPass[BUFLEN_32];
#endif
#ifdef SUPPORT_STUN
   UBOOL8 stunEnable;                          /**< STUN enabled */
   char stunServerAddress[CMS_IPADDR_LENGTH];  /**< IP address of STUN server */
   SINT32 stunServerPort;                      /**< Port number of STUN server */
   char stunUser[BUFLEN_256];                  /**< STUN user name */
   char stunPwd[BUFLEN_256];                   /**< STUN password */
   SINT32 stunMaxKeepAlivePeriod;              /**< Max period for STUN Binding Requests */
   SINT32 stunMinKeepAlivePeriod;              /**< Min period for STUN Binding Requests */
#endif
#ifdef DMP_X_BROADCOM_COM_CONTAINER_1
   char containerName[BUFLEN_64];  /**< Container name */
#endif
#ifdef DMP_DEVICE2_ETHLAG_1
   UBOOL8 enbWanEthLag;                /**< if TRUE, WAN Ethernet LAG enabled */
   char ethLagIfName[BUFLEN_256];      /** < ethLag intf name lists for remove */
   char ethIfName1[BUFLEN_32];         /**< The 1st ethernt layer 2 interface name of LAG */
   char ethIfName2[BUFLEN_32];         /**< The 2nd ethernt layer 2 interface name of LAG */
   char ethLagMode[BUFLEN_64];         /**< mode = 0, (rr), 2 (xor), 4 (802.3ab)  */
   char xmitHashPolicy[BUFLEN_64];     /**< ransmit hash policy to use for slave selection in balance-xor, 802.3ad, default is encap3+4 (4) */
   char lacp_rate[BUFLEN_64];          /**< Option specifying the rate in which we'll ask our link partner to transmit LACPDU packets */
                                       /**< in 802.3ad mode, where 0 is slow (default) 1 is fast */
   SINT32 miimon;                      /**< MII link monitoring frequency in milliseconds.  default is 0 */
#endif

} WEB_NTWK_VAR, *PWEB_NTWK_VAR;


#define VLANMUX_DISABLE -1
#define VLANMUX_MATCH_ANY -2

/* TODO: no limit for pppoe and mer ? */
#define NUM_OF_ALLOWED_SVC_PPPOE 4
#define NUM_OF_ALLOWED_SVC_MER   1
#define NUM_OF_ALLOWED_SVC_BR    1

typedef struct {
   SINT32  vpi;
   SINT32  vci;
   SINT32  port;
   SINT32  conId;
} PORT_VPI_VCI_TUPLE, *PPORT_VPI_VCI_TUPLE;


typedef struct {
   char layer3Name[CMS_IFNAME_LENGTH];
   char protocolStr[BUFLEN_16];
} DIAG_ID, *PDIAG_ID;

/** Generic name list structure
 */

typedef struct _NameList
{
   char              *name;
   struct _NameList  *next;
} NameList;


#include "dal_wan.h"
#include "dal_dsl.h"
#include "dal_lan.h"
#include "dal_dns.h"
#include "dal_moca.h"
#include "dal_cellular.h"


/** Fill in all version and device info fields in the WEB_NTWK_VAR.
 *
 * @param (OUT) pointer to WEB_NTWK_VAR
 */
void cmsDal_getAllVersionInfo(WEB_NTWK_VAR *webVar);


/** Fill in all fields in the WEB_NTWK_VAR.
 *
 * This function was formerly called BcmWeb_getAllInfo.
 *
 * @param (OUT) pointer to WEB_NTWK_VAR
 */
void cmsDal_getAllInfo(WEB_NTWK_VAR *webVar);


/** Get current NAT information.
 *
 * @param varValue   (IN) In not NULL, will be filled with "1" or "0"
 */
CmsRet cmsDal_getEnblNatForWeb(char *varValue);

CmsRet cmsDal_getEnblNatForWeb_igd(char *varValue);

CmsRet cmsDal_getEnblNatForWeb_dev2(char *varValue);


#if defined(SUPPORT_DM_LEGACY98)
#define cmsDal_getEnblNatForWeb(v)       cmsDal_getEnblNatForWeb_igd((v))
#elif defined(SUPPORT_DM_HYBRID)
#define cmsDal_getEnblNatForWeb(v)       cmsDal_getEnblNatForWeb_igd((v))
#elif defined(SUPPORT_DM_PURE181)
#define cmsDal_getEnblNatForWeb(v)       cmsDal_getEnblNatForWeb_dev2((v))
#elif defined(SUPPORT_DM_DETECT)
#define cmsDal_getEnblNatForWeb(v)       (cmsMdm_isDataModelDevice2() ? \
                                     cmsDal_getEnblNatForWeb_dev2((v)) : \
                                     cmsDal_getEnblNatForWeb_igd((v)))
#endif



/** Get current Full Cone NAT information.
 *
 * @param varValue   (IN) In not NULL, will be filled with "1" or "0"
 */
CmsRet cmsDal_getEnblFullconeForWeb(char *varValue);


/** This function returns the interface tr98 full path name from the interface linux name.
 *
 * @param intfname   (IN) the interface linux name.
 * @param layer2     (IN) boolean to indicate whether intfname is a layer 2 or layer 3 interface name.
 * @param mdmPath    (OUT)the interface tr98 full path name. caller shall free the memory after used.
 * @return CmsRet         enum.
 */
CmsRet cmsDal_intfnameToFullPath(const char *intfname, UBOOL8 layer2, char **mdmPath);

/** This function returns the interface linux name from the interface tr98 full path name.
 *
 * @param mdmPath    (IN) the interface tr98 full path name.
 * @param intfname   (OUT)the interface linux name. caller shall have allocated memory for it.
 * @return CmsRet         enum.
 */
CmsRet cmsDal_fullPathToIntfname(const char *mdmPath, char *intfname);


/** Get the current syslog config info from MDM and fill in appropriate
 *  fields in webVar.
 *
 * @param (OUT) pointer to WEB_NTWK_VAR
 *
 * @return CmsRet enum.
 */
CmsRet cmsDal_getCurrentSyslogCfg(WEB_NTWK_VAR *webVar);


/** Call appropriate cmsObj_set function to set the syslog config info.
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 *
 * @return CmsRet enum.
 */
CmsRet cmsDal_setSyslogCfg(const WEB_NTWK_VAR *webVar);


/** Get the current login info from MDM and fill in appropriate
 *  fields in webVar.
 *
 * @param (OUT) pointer to WEB_NTWK_VAR
 *
 * @return CmsRet enum.
 */
CmsRet cmsDal_getCurrentLoginCfg(WEB_NTWK_VAR *webVar);


/** Call appropriate cmsObj_set function to set the loginCfg related info.
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 *
 * @return CmsRet enum.
 */
CmsRet cmsDal_setLoginCfg(const WEB_NTWK_VAR *webVar);




/** Get info from the TR69 ManagementServer object.
 *
 * @param webVar (OUT) webvar variable to be filled in.
 */
void cmsDal_getTr69cCfg(WEB_NTWK_VAR *webVar);

void cmsDal_getTr69cCfg_igd(WEB_NTWK_VAR *webVar);

void cmsDal_getTr69cCfg_dev2(WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define cmsDal_getTr69cCfg(w)       cmsDal_getTr69cCfg_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define cmsDal_getTr69cCfg(w)       cmsDal_getTr69cCfg_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define cmsDal_getTr69cCfg(w)       cmsDal_getTr69cCfg_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define cmsDal_getTr69cCfg(w)       (cmsMdm_isDataModelDevice2() ? \
                                     cmsDal_getTr69cCfg_dev2((w)) : \
                                     cmsDal_getTr69cCfg_igd((w)))
#endif




/** Set the TR69 ManagementServer object.
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 *
 * @return CmsRet enum.
 */
CmsRet cmsDal_setTr69cCfg(const WEB_NTWK_VAR *webVar);

CmsRet cmsDal_setTr69cCfg_igd(const WEB_NTWK_VAR *webVar);

CmsRet cmsDal_setTr69cCfg_dev2(const WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define cmsDal_setTr69cCfg(w)       cmsDal_setTr69cCfg_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define cmsDal_setTr69cCfg(w)       cmsDal_setTr69cCfg_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define cmsDal_setTr69cCfg(w)       cmsDal_setTr69cCfg_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define cmsDal_setTr69cCfg(w)       (cmsMdm_isDataModelDevice2() ? \
                                     cmsDal_setTr69cCfg_dev2((w)) : \
                                     cmsDal_setTr69cCfg_igd((w)))
#endif




/** Get the list of potential/candidate IPv4 default gateways (DefaultConnectionServices)
 *
 * @param gwList (OUT) List of Linux interface names, separated by comma.
 *                     The caller must pass in a buffer of at least
 *                     (CMS_MAX_DEFAULT_GATEWAY * CMS_IFNAME_LENGTH) bytes.
 *
 * @return CmsRet enum.
 */
CmsRet dalRt_getDefaultGatewayList(char *gwIfNames);
CmsRet dalRt_getDefaultGatewayList_igd(char *gwIfNames);
CmsRet dalRt_getDefaultGatewayList_dev2(char *gwIfNames);

#if defined(SUPPORT_DM_LEGACY98)
#define dalRt_getDefaultGatewayList(w)   dalRt_getDefaultGatewayList_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define dalRt_getDefaultGatewayList(w)   dalRt_getDefaultGatewayList_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define dalRt_getDefaultGatewayList(w)   dalRt_getDefaultGatewayList_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define dalRt_getDefaultGatewayList(w)       (cmsMdm_isDataModelDevice2() ? \
                                     dalRt_getDefaultGatewayList_dev2((w)) : \
                                     dalRt_getDefaultGatewayList_igd((w)))
#endif



/** Set the list of potential/candidate IPv4 default gateways (DefaultConnectionServices)
 *
 * @param gwList (IN) List of Linux interface names, separated by comma.
 *                    A max of CMS_MAX_DEFAULT_GATEWAY interface names is supported.
 *
 * @return CmsRet enum.
 */
CmsRet dalRt_setDefaultGatewayList(const char *gwList);
CmsRet dalRt_setDefaultGatewayList_igd(const char *gwList);
CmsRet dalRt_setDefaultGatewayList_dev2(const char *gwList);

#if defined(SUPPORT_DM_LEGACY98)
#define dalRt_setDefaultGatewayList(w)   dalRt_setDefaultGatewayList_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define dalRt_setDefaultGatewayList(w)   dalRt_setDefaultGatewayList_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define dalRt_setDefaultGatewayList(w)   dalRt_setDefaultGatewayList_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define dalRt_setDefaultGatewayList(w)       (cmsMdm_isDataModelDevice2() ? \
                                     dalRt_setDefaultGatewayList_dev2((w)) : \
                                     dalRt_setDefaultGatewayList_igd((w)))
#endif



/** Get the active default gateway interface (the one is used in the system)
 *
 * @param varValue (OUT) The linux user friendly wan interface name.
 *
 * @return CmsRet enum.
 */
CmsRet dalRt_getActiveDefaultGateway(char *varValue);

/** Get the active default gateway IP (the one is used in the system)
 *
 * @param varValue (OUT) The IP address of active Gateway
 *
 * @return CmsRet enum.
 */
CmsRet dalRt_getDefaultGatewayIP(char *varValue);




/** add a static route entry.
 *
 * @param addr      (IN) destination IP address
 * @param mask     (IN) destination subnet mask
 * @param gateway (IN) default gateway IP address of this static route
 * @param wanif     (IN) default interface of this static route
 * @param metric  (IN) hop number to the destination.  Pass in an empty
 *                     string to let the kernel set the metric, otherwise,
 *                     pass in an integer, in the form of a string, to
 *                     specify the metric.
 *
 * @return CmsRet enum.
 */
CmsRet dalStaticRoute_addEntry(const char* addr, const char *mask, const char *gateway, const char *wanif, const char *metric);

CmsRet dalStaticRoute_addEntry_igd(const char* addr, const char *mask, const char *gateway, const char *wanif, const char *metric);

CmsRet dalStaticRoute_addEntry_dev2(const char* addr, const char *mask, const char *gateway, const char *wanif, const char *metric);

#if defined(SUPPORT_DM_LEGACY98)
#define dalStaticRoute_addEntry(addr, mask, gateway, wanif, metric)  dalStaticRoute_addEntry_igd((addr),(mask),(gateway),(wanif),(metric))
#elif defined(SUPPORT_DM_HYBRID)
#define dalStaticRoute_addEntry(addr, mask, gateway, wanif, metric)  dalStaticRoute_addEntry_igd((addr),(mask),(gateway),(wanif),(metric))
#elif defined(SUPPORT_DM_PURE181)
#define dalStaticRoute_addEntry(addr, mask, gateway, wanif, metric)  dalStaticRoute_addEntry_dev2((addr),(mask),(gateway),(wanif),(metric))
#elif defined(SUPPORT_DM_DETECT)
#define dalStaticRoute_addEntry(addr, mask, gateway, wanif, metric)  (cmsMdm_isDataModelDevice2() ? \
                                                                         dalStaticRoute_addEntry_dev2((addr),(mask),(gateway),(wanif),(metric)) : \
                                                                         dalStaticRoute_addEntry_igd((addr),(mask),(gateway),(wanif),(metric)))
#endif


/** delete a static route entry.
 *
 * @param addr      (IN) destination IP address
 * @param mask     (IN) destination subnet mask
 *
 * @return CmsRet enum.
 **/
CmsRet dalStaticRoute_deleteEntry(const char* addr, const char *mask);

CmsRet dalStaticRoute_deleteEntry_igd(const char* addr, const char *mask);

CmsRet dalStaticRoute_deleteEntry_dev2(const char* addr, const char *mask);

#if defined(SUPPORT_DM_LEGACY98)
#define dalStaticRoute_deleteEntry(addr,mask)  dalStaticRoute_deleteEntry_igd((addr),(mask))
#elif defined(SUPPORT_DM_HYBRID)
#define dalStaticRoute_deleteEntry(addr,mask)  dalStaticRoute_deleteEntry_igd((addr),(mask))
#elif defined(SUPPORT_DM_PURE181)
#define dalStaticRoute_deleteEntry(addr,mask)  dalStaticRoute_deleteEntry_dev2((addr),(mask))
#elif defined(SUPPORT_DM_DETECT)
#define dalStaticRoute_deleteEntry(addr,mask)  (cmsMdm_isDataModelDevice2() ? \
                                                                         dalStaticRoute_deleteEntry_dev2((addr),(mask)) : \
                                                                         dalStaticRoute_deleteEntry_igd((addr),(mask)))
#endif


#ifdef SUPPORT_POLICYROUTING
/** add a policy route entry.
 *
 *
 * @return CmsRet enum.
 */
CmsRet dalPolicyRouting_addEntry(const char* PolicyName, const char *SourceIp, const char *SourceIfName, const char *wanif, const char *defaultGW);
CmsRet dalPolicyRouting_addEntry_igd(const char* PolicyName, const char *SourceIp, const char *SourceIfName, const char *wanif, const char *defaultGW);
CmsRet dalPolicyRouting_addEntry_dev2(const char* PolicyName, const char *SourceIp, const char *SourceIfName, const char *wanif, const char *defaultGW);

#if defined(SUPPORT_DM_LEGACY98)
#define dalPolicyRouting_addEntry(PolicyName,SourceIp,SourceIfName,wanif,wanifdefaultGW)  dalPolicyRouting_addEntry_igd((PolicyName),(SourceIp),(SourceIfName),(wanif),(wanifdefaultGW))
#elif defined(SUPPORT_DM_HYBRID)
#define dalPolicyRouting_addEntry(PolicyName,SourceIp,SourceIfName,wanif,wanifdefaultGW)  dalPolicyRouting_addEntry_igd((PolicyName),(SourceIp),(SourceIfName),(wanif),(wanifdefaultGW))
#elif defined(SUPPORT_DM_PURE181)
#define dalPolicyRouting_addEntry(PolicyName,SourceIp,SourceIfName,wanif,wanifdefaultGW)  dalPolicyRouting_addEntry_dev2((PolicyName),(SourceIp),(SourceIfName),(wanif),(wanifdefaultGW))
#elif defined(SUPPORT_DM_DETECT)
#define dalPolicyRouting_addEntry(PolicyName,SourceIp,SourceIfName,wanif,wanifdefaultGW)  (cmsMdm_isDataModelDevice2() ? \
                                                                         dalPolicyRouting_addEntry_dev2((PolicyName),(SourceIp),(SourceIfName),(wanif),(wanifdefaultGW)) : \
                                                                         dalPolicyRouting_addEntry_igd((PolicyName),(SourceIp),(SourceIfName),(wanif),(wanifdefaultGW)))
#endif

/** delete a policy route entry.
 *
 *
 * @return CmsRet enum.
 */
CmsRet dalPolicyRouting_deleteEntry(const char* PolicyName);
CmsRet dalPolicyRouting_deleteEntry_igd(const char* PolicyName);
CmsRet dalPolicyRouting_deleteEntry_dev2(const char* PolicyName);

#if defined(SUPPORT_DM_LEGACY98)
#define dalPolicyRouting_deleteEntry(PolicyName)  dalPolicyRouting_deleteEntry_igd((PolicyName))
#elif defined(SUPPORT_DM_HYBRID)
#define dalPolicyRouting_deleteEntry(PolicyName)  dalPolicyRouting_deleteEntry_igd((PolicyName))
#elif defined(SUPPORT_DM_PURE181)
#define dalPolicyRouting_deleteEntry(PolicyName)  dalPolicyRouting_deleteEntry_dev2((PolicyName))
#elif defined(SUPPORT_DM_DETECT)
#define dalPolicyRouting_deleteEntry(PolicyName)  (cmsMdm_isDataModelDevice2() ? \
                                                 dalPolicyRouting_deleteEntry_dev2((PolicyName)) : \
                                                 dalPolicyRouting_deleteEntry_igd((PolicyName)))
#endif

#endif /* SUPPORT_POLICYROUTING */


/** add a virtual server entry.
 *
 * @param dstWanIf (IN) virtual server of which WAN interface
 * @param srvName (IN) virtual server service name
 * @param srvAddr  (IN) virtual server IP address
 * @param protocol  (IN) virtual server service protocol
 * @param EPS        (IN) virtual server external port start
 * @param EPE        (IN) virtual server external port end
 * @param IPS        (IN) virtual server internal port start
 * @param IPE        (IN) virtual server internal port end
 *
 * @return CmsRet enum.
 */
CmsRet dalVirtualServer_addEntry(const char *dstWanIf, const char *srvName, const char *srvAddr, const char *protocol, const UINT16 EPS, const UINT16 EPE, const UINT16 IPS, const UINT16 IPE);

CmsRet dalVirtualServer_addEntry_igd(const char *dstWanIf, const char *srvName, const char *srvAddr, const char *protocol, const UINT16 EPS, const UINT16 EPE, const UINT16 IPS, const UINT16 IPE);

CmsRet dalVirtualServer_addEntry_dev2(const char *dstWanIf, const char *srvName, const char *srvAddr, const char *protocol, const UINT16 EPS, const UINT16 EPE, const UINT16 IPS, const UINT16 IPE);

#if defined(SUPPORT_DM_LEGACY98)
#define dalVirtualServer_addEntry(dstW,sN,sA,proto,EPS,EPE,IPS,IPE)  dalVirtualServer_addEntry_igd((dstW),(sN),(sA),(proto),(EPS),(EPE),(IPS),(IPE))
#elif defined(SUPPORT_DM_HYBRID)
#define dalVirtualServer_addEntry(dstW,sN,sA,proto,EPS,EPE,IPS,IPE)  dalVirtualServer_addEntry_igd((dstW),(sN),(sA),(proto),(EPS),(EPE),(IPS),(IPE))
#elif defined(SUPPORT_DM_PURE181)
#define dalVirtualServer_addEntry(dstW,sN,sA,proto,EPS,EPE,IPS,IPE)  dalVirtualServer_addEntry_dev2((dstW),(sN),(sA),(proto),(EPS),(EPE),(IPS),(IPE))
#elif defined(SUPPORT_DM_DETECT)
#define dalVirtualServer_addEntry(dstW,sN,sA,proto,EPS,EPE,IPS,IPE)  (cmsMdm_isDataModelDevice2() ? \
                                                                         dalVirtualServer_addEntry_dev2((dstW),(sN),(sA),(proto),(EPS),(EPE),(IPS),(IPE)) : \
                                                                         dalVirtualServer_addEntry_igd((dstW),(sN),(sA),(proto),(EPS),(EPE),(IPS),(IPE)))
#endif


/** deleter a virtual server entry.
 *
 * @param srvAddr  (IN) virtual server IP address
 * @param protocol  (IN) virtual server service protocol
 * @param EPS        (IN) virtual server external port start
 * @param EPE        (IN) virtual server external port end
 * @param IPS        (IN) virtual server internal port start
 * @param IPE        (IN) virtual server internal port end
 *
 * @return CmsRet enum.
 */
CmsRet dalVirtualServer_deleteEntry(const char * srvAddr, const char * protocol, const UINT16 EPS, const UINT16 EPE, const UINT16 IPS, const UINT16 IPE);

CmsRet dalVirtualServer_deleteEntry_igd(const char * srvAddr, const char * protocol, const UINT16 EPS, const UINT16 EPE, const UINT16 IPS, const UINT16 IPE);

CmsRet dalVirtualServer_deleteEntry_dev2(const char * srvAddr, const char * protocol, const UINT16 EPS, const UINT16 EPE, const UINT16 IPS, const UINT16 IPE);

#if defined(SUPPORT_DM_LEGACY98)
#define dalVirtualServer_deleteEntry(sA,proto,EPS,EPE,IPS,IPE)  dalVirtualServer_deleteEntry_igd((sA),(proto),(EPS),(EPE),(IPS),(IPE))
#elif defined(SUPPORT_DM_HYBRID)
#define dalVirtualServer_deleteEntry(sA,proto,EPS,EPE,IPS,IPE)  dalVirtualServer_deleteEntry_igd((sA),(proto),(EPS),(EPE),(IPS),(IPE))
#elif defined(SUPPORT_DM_PURE181)
#define dalVirtualServer_deleteEntry(sA,proto,EPS,EPE,IPS,IPE)  dalVirtualServer_deleteEntry_dev2((sA),(proto),(EPS),(EPE),(IPS),(IPE))
#elif defined(SUPPORT_DM_DETECT)
#define dalVirtualServer_deleteEntry(sA,proto,EPS,EPE,IPS,IPE)  (cmsMdm_isDataModelDevice2() ? \
                                                                         dalVirtualServer_deleteEntry_dev2((sA),(proto),(EPS),(EPE),(IPS),(IPE)) : \
                                                                         dalVirtualServer_deleteEntry_igd((sA),(proto),(EPS),(EPE),(IPS),(IPE)))
#endif


/** add a Dmz host entry.
 *
 * @param addr      (IN) DMZ host IP address
 *
 * @return CmsRet enum.
 */
CmsRet dalDmzHost_addEntry(const char *srvAddr);

CmsRet dalDmzHost_addEntry_igd(const char *srvAddr);

CmsRet dalDmzHost_addEntry_dev2(const char *srvAddr);

#if defined(SUPPORT_DM_LEGACY98)
#define dalDmzHost_addEntry(addr)  dalDmzHost_addEntry_igd((addr))
#elif defined(SUPPORT_DM_HYBRID)
#define dalDmzHost_addEntry(addr)  dalDmzHost_addEntry_igd((addr))
#elif defined(SUPPORT_DM_PURE181)
#define dalDmzHost_addEntry(addr)  dalDmzHost_addEntry_dev2((addr))
#elif defined(SUPPORT_DM_DETECT)
#define dalDmzHost_addEntry(addr)  (cmsMdm_isDataModelDevice2() ? \
                               dalDmzHost_addEntry_igd((addr)) : \
                               dalDmzHost_addEntry_dev2((addr)))
#endif



/** get Dmz host information.
 *
 * @param address      (IN) IP address
 *
 * @return CmsRet enum.
 */
CmsRet dalGetDmzHost(char *address);

CmsRet dalGetDmzHost_igd(char *address);

CmsRet dalGetDmzHost_dev2(char *address);

#if defined(SUPPORT_DM_LEGACY98)
#define dalGetDmzHost(addr)  dalGetDmzHost_igd((addr))
#elif defined(SUPPORT_DM_HYBRID)
#define dalGetDmzHost(addr)  dalGetDmzHost_igd((addr))
#elif defined(SUPPORT_DM_PURE181)
#define dalGetDmzHost(addr)  dalGetDmzHost_dev2((addr))
#elif defined(SUPPORT_DM_DETECT)
#define dalGetDmzHost(addr)  (cmsMdm_isDataModelDevice2() ? \
                               dalGetDmzHost_igd((addr)) : \
                               dalGetDmzHost_dev2((addr)))
#endif


/** add a port triggering entry.
 *
 * @param dstWanIf (IN) port triggering of which WAN interface
 * @param appName (IN) port triggering application name
 * @param tProto  (IN) trigger protocol
 * @param oProto  (IN) open protocol
 * @param tPS        (IN) trigger port start
 * @param tPE        (IN) trigger port end
 * @param oPS        (IN) open port start
 * @param oPE        (IN) open port end
 *
 * @return CmsRet enum.
 */
CmsRet dalPortTriggering_addEntry(const char *dstWanIf, const char *appName,
   const char *tProto, const char *oProto, const UINT16 tPS,
   const UINT16 tPE, const UINT16 oPS, const UINT16 oPE);

CmsRet dalPortTriggering_addEntry_igd(const char *dstWanIf, const char *appName,
   const char *tProto, const char *oProto, const UINT16 tPS,
   const UINT16 tPE, const UINT16 oPS, const UINT16 oPE);

CmsRet dalPortTriggering_addEntry_dev2(const char *dstWanIf, const char *appName,
   const char *tProto, const char *oProto, const UINT16 tPS,
   const UINT16 tPE, const UINT16 oPS, const UINT16 oPE);


#if defined(SUPPORT_DM_LEGACY98)
#define dalPortTriggering_addEntry(dWIf,apN,tPro,oPro,tPS,tPE,oPS,oPE)  dalPortTriggering_addEntry_igd((dWIf),(apN),(tPro),(oPro),(tPS),(tPE),(oPS),(oPE))
#elif defined(SUPPORT_DM_HYBRID)
#define dalPortTriggering_addEntry(dWIf,apN,tPro,oPro,tPS,tPE,oPS,oPE)  dalPortTriggering_addEntry_igd((dWIf),(apN),(tPro),(oPro),(tPS),(tPE),(oPS),(oPE))
#elif defined(SUPPORT_DM_PURE181)
#define dalPortTriggering_addEntry(dWIf,apN,tPro,oPro,tPS,tPE,oPS,oPE)  dalPortTriggering_addEntry_dev2((dWIf),(apN),(tPro),(oPro),(tPS),(tPE),(oPS),(oPE))
#elif defined(SUPPORT_DM_DETECT)
#define dalPortTriggering_addEntry(dWIf,apN,tPro,oPro,tPS,tPE,oPS,oPE)  (cmsMdm_isDataModelDevice2() ? \
                               dalPortTriggering_addEntry_dev2((dWIf),(apN),(tPro),(oPro),(tPS),(tPE),(oPS),(oPE)) : \
                               dalPortTriggering_addEntry_igd((dWIf),(apN),(tPro),(oPro),(tPS),(tPE),(oPS),(oPE)))
#endif


/** add a port triggering entry.
 *
 * @param dstWanIf (IN) port triggering of which WAN interface
 * @param tProto  (IN) trigger protocol
 * @param tPS        (IN) trigger port start
 * @param tPE        (IN) trigger port end
 *
 * @return CmsRet enum.
 */
CmsRet dalPortTriggering_deleteEntry(const char *dstWanIf, const char *tProto,
   const UINT16 tPS, const UINT16 tPE, const UINT16 tOS, const UINT16 tOE);

CmsRet dalPortTriggering_deleteEntry_igd(const char *dstWanIf, const char *tProto,
   const UINT16 tPS, const UINT16 tPE, const UINT16 tOS, const UINT16 tOE);

CmsRet dalPortTriggering_deleteEntry_dev2(const char *dstWanIf, const char *tProto,
   const UINT16 tPS, const UINT16 tPE, const UINT16 tOS, const UINT16 tOE);

#if defined(SUPPORT_DM_LEGACY98)
#define dalPortTriggering_deleteEntry(dWIf,tPro,tPS,tPE,tOS,tOE)  dalPortTriggering_deleteEntry_igd((dWIf),(tPro),(tPS),(tPE),(tOS),(tOE))
#elif defined(SUPPORT_DM_HYBRID)
#define dalPortTriggering_deleteEntry(dWIf,tPro,tPS,tPE,tOS,tOE)  dalPortTriggering_deleteEntry_igd((dWIf),(tPro),(tPS),(tPE),(tOS),(tOE))
#elif defined(SUPPORT_DM_PURE181)
#define dalPortTriggering_deleteEntry(dWIf,tPro,tPS,tPE,tOS,tOE)  dalPortTriggering_deleteEntry_dev2((dWIf),(tPro),(tPS),(tPE),(tOS),(tOE))
#elif defined(SUPPORT_DM_DETECT)
#define dalPortTriggering_deleteEntry(dWIf,tPro,tPS,tPE,tOS,tOE)  (cmsMdm_isDataModelDevice2() ? \
                               dalPortTriggering_deleteEntry_dev2((dWIf),(tPro),(tPS),(tPE),(tOS),(tOE)) : \
                               dalPortTriggering_deleteEntry_igd((dWIf),(tPro),(tPS),(tPE),(tOS),(tOE)))
#endif


CmsRet dalNat_deletePortMapping_dev2(const char *ipIntfFullPath);
CmsRet dalNat_deleteIntfSetting_dev2(const char * ipIntffullPath);


/** add a IP filter in entry.
 *
 * @param name          (IN) IP filter name
 * @param ipver         (IN) IP filter ip version
 * @param protocol      (IN) IP filter protocol
 * @param srcAddr       (IN) IP filter source IP address
 * @param srcMask      (IN) IP filter source netmask
 * @param srcPort        (IN) IP filter source port number
 * @param dstAddr       (IN) IP filter destination IP address
 * @param dstMask      (IN) IP filter destination netmask
 * @param dstPort        (IN) IP filter destination port number
 * @param ifName        (IN) wan interface for IP filter in rule
 *
 * @return CmsRet enum.
 */
CmsRet dalSec_addIpFilterIn(const char *name, const char *ipver, const char *protocol,
                            const char *srcAddr, const char *srcMask, const char *srcPort,
                            const char *dstAddr, const char *dstMask, const char *dstPort, const char *ifName);

CmsRet dalSec_addIpFilterIn_igd(const char *name, const char *ipver, const char *protocol,
                            const char *srcAddr, const char *srcMask, const char *srcPort,
                            const char *dstAddr, const char *dstMask, const char *dstPort, const char *ifName);


CmsRet dalSec_addIpFilterIn_dev2(const char *name, const char *ipver, const char *protocol,
                            const char *srcAddr, const char *srcMask, const char *srcPort,
                            const char *dstAddr, const char *dstMask, const char *dstPort, const char *ifName);


#if defined(SUPPORT_DM_LEGACY98)
#define dalSec_addIpFilterIn(n,i,p,sa,sm,sp,da,dm,dp,ifn)  dalSec_addIpFilterIn_igd((n),(i),(p),(sa),(sm),(sp),(da),(dm),(dp),(ifn))
#elif defined(SUPPORT_DM_HYBRID)
#define dalSec_addIpFilterIn(n,i,p,sa,sm,sp,da,dm,dp,ifn)  dalSec_addIpFilterIn_igd((n),(i),(p),(sa),(sm),(sp),(da),(dm),(dp),(ifn))
#elif defined(SUPPORT_DM_PURE181)
#define dalSec_addIpFilterIn(n,i,p,sa,sm,sp,da,dm,dp,ifn)  dalSec_addIpFilterIn_dev2((n),(i),(p),(sa),(sm),(sp),(da),(dm),(dp),(ifn))
#elif defined(SUPPORT_DM_DETECT)
#define dalSec_addIpFilterIn(n,i,p,sa,sm,sp,da,dm,dp,ifn)  (cmsMdm_isDataModelDevice2() ? \
                               dalSec_addIpFilterIn_dev2((n),(i),(p),(sa),(sm),(sp),(da),(dm),(dp),(ifn)) : \
                               dalSec_addIpFilterIn_igd((n),(i),(p),(sa),(sm),(sp),(da),(dm),(dp),(ifn)))
#endif


/** delete a IP filter in entry.
 *
 * @param name      (IN) IP filter name
 *
 * @return CmsRet enum.
 **/
CmsRet dalSec_deleteIpFilterIn(const char* name);

CmsRet dalSec_deleteIpFilterIn_igd(const char* name);

CmsRet dalSec_deleteIpFilterIn_dev2(const char* name);

#if defined(SUPPORT_DM_LEGACY98)
#define dalSec_deleteIpFilterIn(i)  dalSec_deleteIpFilterIn_igd((i))
#elif defined(SUPPORT_DM_HYBRID)
#define dalSec_deleteIpFilterIn(i)  dalSec_deleteIpFilterIn_igd((i))
#elif defined(SUPPORT_DM_PURE181)
#define dalSec_deleteIpFilterIn(i)  dalSec_deleteIpFilterIn_dev2((i))
#elif defined(SUPPORT_DM_DETECT)
#define dalSec_deleteIpFilterIn(i)  (cmsMdm_isDataModelDevice2() ? \
                               dalSec_deleteIpFilterIn_dev2((i)) : \
                               dalSec_deleteIpFilterIn_igd((i)))
#endif


/** add a IP filter out entry.
 *
 * @param name          (IN) IP filter name
 * @param ipver         (IN) IP filter ip version
 * @param protocol      (IN) IP filter protocol
 * @param srcAddr       (IN) IP filter source IP address
 * @param srcMask      (IN) IP filter source netmask
 * @param srcPort        (IN) IP filter source port number
 * @param dstAddr       (IN) IP filter destination IP address
 * @param dstMask      (IN) IP filter destination netmask
 * @param dstPort        (IN) IP filter destination port number
 * @param ifName        (IN) wan interface for IP filter out rule
 *
 * @return CmsRet enum.
 */
CmsRet dalSec_addIpFilterOut(const char *name, const char *ipver, const char *protocol,
                             const char *srcAddr, const char *srcMask, const char *srcPort,
                             const char *dstAddr, const char *dstMask, const char *dstPort, const char *ifName);

CmsRet dalSec_addIpFilterOut_igd(const char *name, const char *ipver, const char *protocol,
                             const char *srcAddr, const char *srcMask, const char *srcPort,
                             const char *dstAddr, const char *dstMask, const char *dstPort, const char *ifName);

CmsRet dalSec_addIpFilterOut_dev2(const char *name, const char *ipver, const char *protocol,
                             const char *srcAddr, const char *srcMask, const char *srcPort,
                             const char *dstAddr, const char *dstMask, const char *dstPort, const char *ifName);

#if defined(SUPPORT_DM_LEGACY98)
#define dalSec_addIpFilterOut(n,i,p,sa,sm,sp,da,dm,dp,ifn)  dalSec_addIpFilterOut_igd((n),(i),(p),(sa),(sm),(sp),(da),(dm),(dp),(ifn))
#elif defined(SUPPORT_DM_HYBRID)
#define dalSec_addIpFilterOut(n,i,p,sa,sm,sp,da,dm,dp,ifn)  dalSec_addIpFilterOut_igd((n),(i),(p),(sa),(sm),(sp),(da),(dm),(dp),(ifn))
#elif defined(SUPPORT_DM_PURE181)
#define dalSec_addIpFilterOut(n,i,p,sa,sm,sp,da,dm,dp,ifn)  dalSec_addIpFilterOut_dev2((n),(i),(p),(sa),(sm),(sp),(da),(dm),(dp),(ifn))
#elif defined(SUPPORT_DM_DETECT)
#define dalSec_addIpFilterOut(n,i,p,sa,sm,sp,da,dm,dp,ifn)  (cmsMdm_isDataModelDevice2() ? \
                               dalSec_addIpFilterOut_dev2((n),(i),(p),(sa),(sm),(sp),(da),(dm),(dp),(ifn)) : \
                               dalSec_addIpFilterOut_igd((n),(i),(p),(sa),(sm),(sp),(da),(dm),(dp),(ifn)))
#endif



/** delete a IP filter out entry.
 *
 * @param name      (IN) IP filter name
 *
 * @return CmsRet enum.
 **/
CmsRet dalSec_deleteIpFilterOut(const char* name);

CmsRet dalSec_deleteIpFilterOut_igd(const char* name);

CmsRet dalSec_deleteIpFilterOut_dev2(const char* name);


#if defined(SUPPORT_DM_LEGACY98)
#define dalSec_deleteIpFilterOut(i)  dalSec_deleteIpFilterOut_igd((i))
#elif defined(SUPPORT_DM_HYBRID)
#define dalSec_deleteIpFilterOut(i)  dalSec_deleteIpFilterOut_igd((i))
#elif defined(SUPPORT_DM_PURE181)
#define dalSec_deleteIpFilterOut(i)  dalSec_deleteIpFilterIn_dev2((i))
#elif defined(SUPPORT_DM_DETECT)
#define dalSec_deleteIpFilterOut(i)  (cmsMdm_isDataModelDevice2() ? \
                               dalSec_deleteIpFilterOut_dev2((i)) : \
                               dalSec_deleteIpFilterOut_igd((i)))
#endif


/** Add an IPSec tunnel entry.
 *
 * @param name      (IN) pWebVar
 *
 * @return CmsRet enum.
 **/
CmsRet dalIPSec_addTunnel(const PWEB_NTWK_VAR pWebVar);
CmsRet dalIPSec_addTunnel_igd(const PWEB_NTWK_VAR pWebVar);
CmsRet dalIPSec_addTunnel_dev2(const PWEB_NTWK_VAR pWebVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalIPSec_addTunnel(i)  dalIPSec_addTunnel_igd((i))
#elif defined(SUPPORT_DM_HYBRID)
#define dalIPSec_addTunnel(i)  dalIPSec_addTunnel_igd((i))
#elif defined(SUPPORT_DM_PURE181)
#define dalIPSec_addTunnel(i)  dalIPSec_addTunnel_dev2((i))
#elif defined(SUPPORT_DM_DETECT)
#define dalIPSec_addTunnel(i)  (cmsMdm_isDataModelDevice2() ? \
                               dalIPSec_addTunnel_dev2((i)) : \
                               dalIPSec_addTunnel_igd((i)))
#endif


/** delete an IPSec tunnel entry.
 *
 * @param name      (IN) IPSec tunnel name
 *
 * @return CmsRet enum.
 **/
CmsRet dalIPSec_deleteTunnel(const char* name);
CmsRet dalIPSec_deleteTunnel_igd(const char* name);
CmsRet dalIPSec_deleteTunnel_dev2(const char* name);

#if defined(SUPPORT_DM_LEGACY98)
#define dalIPSec_deleteTunnel(i)  dalIPSec_deleteTunnel_igd((i))
#elif defined(SUPPORT_DM_HYBRID)
#define dalIPSec_deleteTunnel(i)  dalIPSec_deleteTunnel_igd((i))
#elif defined(SUPPORT_DM_PURE181)
#define dalIPSec_deleteTunnel(i)  dalIPSec_deleteTunnel_dev2((i))
#elif defined(SUPPORT_DM_DETECT)
#define dalIPSec_deleteTunnel(i)  (cmsMdm_isDataModelDevice2() ? \
                               dalIPSec_deleteTunnel_dev2((i)) : \
                               dalIPSec_deleteTunnel_igd((i)))
#endif


/** Reset IPSec parameters.
 *
 * @param name      (IN) pWebVar
 *
 * @return CmsRet enum.
 **/
CmsRet dalIPSec_setDefaultValues(PWEB_NTWK_VAR pWebVar);


/** add a Mac filter entry.
 *
 * @param protocol     (IN) Mac filter protocol
 * @param srcMac       (IN) Mac filter source Mac address
 * @param dstMac       (IN) Mac filter destination Mac address
 * @param direction     (IN) Mac filter direction
 * @param ifName       (IN) wan interface for Mac filter rule
 *
 * @return CmsRet enum.
 */
CmsRet dalSec_addMacFilter(const char *protocol, const char *srcMac, const char *dstMac, const char *direction, const char *ifName);

CmsRet dalSec_addMacFilter_igd(const char *protocol, const char *srcMac, const char *dstMac, const char *direction, const char *ifName);

CmsRet dalSec_addMacFilter_dev2(const char *protocol, const char *srcMac, const char *dstMac, const char *direction, const char *ifName);


#if defined(SUPPORT_DM_LEGACY98)
#define dalSec_addMacFilter(proto,sMac,dMac,dir,ifName)  dalSec_addMacFilter_igd((proto),(sMac),(dMac),(dir),(ifName))
#elif defined(SUPPORT_DM_HYBRID)
#define dalSec_addMacFilter(proto,sMac,dMac,dir,ifName)  dalSec_addMacFilter_igd((proto),(sMac),(dMac),(dir),(ifName))
#elif defined(SUPPORT_DM_PURE181)
#define dalSec_addMacFilter(proto,sMac,dMac,dir,ifName)  dalSec_addMacFilter_dev2((proto),(sMac),(dMac),(dir),(ifName))
#elif defined(SUPPORT_DM_DETECT)
#define dalSec_addMacFilter(proto,sMac,dMac,dir,ifName)  (cmsMdm_isDataModelDevice2() ? \
                               dalSec_addMacFilter_dev2((proto),(sMac),(dMac),(dir),(ifName)) : \
                               dalSec_addMacFilter_igd((proto),(sMac),(dMac),(dir),(ifName)))
#endif

#ifdef BRCM_PKTCBL_SUPPORT
/** edit eSafe interface attribute.
 *
 * @param ifName                 (IN) interface name
 * @param isIPv4Enabled       (IN) isIPv4Enabled
 * @param isIPv6Enabled       (IN) isIPv6Enabled
 *
 * @return CmsRet enum.
 **/
CmsRet  dalWan_editeSafe(const char *ifName, UBOOL8 isIPv4Enabled, UBOOL8 isIPv6Enabled);


/** get eSafe interface attribute.
 *
 * @param ifName                 (IN) interface name
 * @param isIPv4Enabled       (OUT) isIPv4Enabled
 * @param isIPv6Enabled       (OUT) isIPv6Enabled
 *
 * @return CmsRet enum.
 **/
CmsRet  dalWan_geteSafe(const char *ifName, UBOOL8 *isIPv4Enabled, UBOOL8 *isIPv6Enabled);

#if defined(SUPPORT_DM_LEGACY98)
#define dalWan_editeSafe(ifName,isIPv4Enabled,isIPv6Enabled)  dalWan_editeSafe_igd((ifName),(isIPv4Enabled),(isIPv6Enabled))
#define dalWan_geteSafe(ifName,isIPv4Enabled,isIPv6Enabled)  dalWan_geteSafe_igd((ifName),(isIPv4Enabled),(isIPv6Enabled))
#endif
#endif // BRCM_PKTCBL_SUPPORT


/** delete a Mac filter entry.
 *
 * @param protocol     (IN) Mac filter protocol
 * @param srcMac       (IN) Mac filter source Mac address
 * @param dstMac       (IN) Mac filter destination Mac address
 * @param direction     (IN) Mac filter direction
 * @param ifName       (IN) wan interface for Mac filter rule
 *
 * @return CmsRet enum.
 **/
CmsRet dalSec_deleteMacFilter(const char *protocol, const char *srcMac, const char *dstMac, const char *direction, const char *ifName);

CmsRet dalSec_deleteMacFilter_igd(const char *protocol, const char *srcMac, const char *dstMac, const char *direction, const char *ifName);

CmsRet dalSec_deleteMacFilter_dev2(const char *protocol, const char *srcMac, const char *dstMac, const char *direction, const char *ifName);


#if defined(SUPPORT_DM_LEGACY98)
#define dalSec_deleteMacFilter(proto,sMac,dMac,dir,ifName)  dalSec_deleteMacFilter_igd((proto),(sMac),(dMac),(dir),(ifName))
#elif defined(SUPPORT_DM_HYBRID)
#define dalSec_deleteMacFilter(proto,sMac,dMac,dir,ifName)  dalSec_deleteMacFilter_igd((proto),(sMac),(dMac),(dir),(ifName))
#elif defined(SUPPORT_DM_PURE181)
#define dalSec_deleteMacFilter(proto,sMac,dMac,dir,ifName)  dalSec_deleteMacFilter_dev2((proto),(sMac),(dMac),(dir),(ifName))
#elif defined(SUPPORT_DM_DETECT)
#define dalSec_deleteMacFilter(proto,sMac,dMac,dir,ifName)  (cmsMdm_isDataModelDevice2() ? \
                               dalSec_deleteMacFilter_dev2((proto),(sMac),(dMac),(dir),(ifName)) : \
                               dalSec_deleteMacFilter_igd((proto),(sMac),(dMac),(dir),(ifName)))
#endif


/** change a Mac filter policy.
 *
 * @param ifName      (IN)
 *
 * @return CmsRet enum.
 **/
CmsRet dalSec_ChangeMacFilterPolicy(const char *ifName);

CmsRet dalSec_ChangeMacFilterPolicy_igd(const char *ifName);

CmsRet dalSec_ChangeMacFilterPolicy_dev2(const char *ifName);



#if defined(SUPPORT_DM_LEGACY98)
#define dalSec_ChangeMacFilterPolicy(ifName)  dalSec_ChangeMacFilterPolicy_igd((ifName))
#elif defined(SUPPORT_DM_HYBRID)
#define dalSec_ChangeMacFilterPolicy(ifName)  dalSec_ChangeMacFilterPolicy_igd((ifName))
#elif defined(SUPPORT_DM_PURE181)
#define dalSec_ChangeMacFilterPolicy(ifName)  dalSec_ChangeMacFilterPolicy_dev2((ifName))
#elif defined(SUPPORT_DM_DETECT)
#define dalSec_ChangeMacFilterPolicy(ifName)  (cmsMdm_isDataModelDevice2() ? \
                               dalSec_ChangeMacFilterPolicy_dev2((ifName)) : \
                               dalSec_ChangeMacFilterPolicy_igd((ifName)))
#endif


/** configure Dns proxy.
 *
 * @param enable          (IN)
 * @param hostname      (IN)
 * @param domainname (IN)
 *
 * @return CmsRet enum.
 */
CmsRet dalDnsProxyCfg(const char *enable, const char *hostname, const char *domainname);


/** get Dns proxy information.
 *
 * @param info      (IN)
 *
 * @return CmsRet enum.
 */
CmsRet dalGetDnsProxy(char *info);


/** add a access time restriction entry.
 *
 * @param username (IN) service name
 * @param mac         (IN) block mac address
 * @param days        (IN) block days
 * @param starttime  (IN) block start time
 * @param endtime   (IN) block end time
 *
 * @return CmsRet enum.
 */
CmsRet dalAccessTimeRestriction_addEntry(const char *username, const char *mac, const unsigned char days, const unsigned short int starttime, const unsigned short int endtime);


/** deleter a access time restriction entry.
 *
 * @param username  (IN) delete service name
 *
 * @return CmsRet enum.
 */
CmsRet dalAccessTimeRestriction_deleteEntry(const char *username);


/** add a url filter list entry.
 *
 * @param url_address  (IN) add list url address
 * @param url_port       (IN) add list port number
 *
 * @return CmsRet enum.
 */
CmsRet dalUrlFilter_addEntry(const char* url_address, const UINT32 url_port);


/** deleter a url filter list entry.
 *
 * @param url_address  (IN) delete url address
 *
 * @return CmsRet enum.
 */
CmsRet dalUrlFilter_removeEntry(const char *url_address);


/** set the type of the url filter list.
 *
 * @param type   (IN) type of url list
 *
 * @return CmsRet enum.
 */
CmsRet dalUrlFilter_setType(const char *type);


/** obtain the type of the url filter list.
 *
 * @param type   (IN)
 *
 * @return CmsRet enum.
 */
CmsRet dalUrlFilter_getType(char *type);




/** Get the network access mode of an application.
 *
 * This interface requires the caller to obtain the cms lock
 *
 * @param eid  (IN) CMS entity id that is trying to access the modem.
 * @param addr (IN) IPv4 address of the entity that is trying to access the modem.
 *
 * @return enum of access modes.
 */
NetworkAccessMode cmsDal_getNetworkAccessMode(CmsEntityId eid, const char *addr);

NetworkAccessMode cmsDal_getNetworkAccessMode_igd(CmsEntityId eid, const char *addr);

NetworkAccessMode cmsDal_getNetworkAccessMode_dev2(CmsEntityId eid, const char *addr);

#if defined(SUPPORT_DM_LEGACY98)
#define cmsDal_getNetworkAccessMode(e, a) cmsDal_getNetworkAccessMode_igd((e), (a))
#elif defined(SUPPORT_DM_HYBRID)
#define cmsDal_getNetworkAccessMode(e, a) cmsDal_getNetworkAccessMode_igd((e), (a))
#elif defined(SUPPORT_DM_PURE181)
#define cmsDal_getNetworkAccessMode(e, a) cmsDal_getNetworkAccessMode_dev2((e), (a))
#elif defined(SUPPORT_DM_DETECT)
#define cmsDal_getNetworkAccessMode(e, a) (cmsMdm_isDataModelDevice2() ? \
                            cmsDal_getNetworkAccessMode_dev2((e), (a)) : \
                            cmsDal_getNetworkAccessMode_igd((e), (a)))
#endif




/** Authenticate given username and password.
 *
 * The algorithm works as follows:
 * If network access mode is LAN_SIDE or CONSOLE, validate against admin or user
 * If network access is from WAN_SIDE, validate against support.
 *
 * This interface requires the caller to obtain the cms lock
 *
 * @param authLevel     (OUT) the login level (user, support and admin)
 * @param accessMode    (IN) the network access mode.
 * @param username      (IN) username
 * @param passwd        (IN) password
 * @param lockTimeoutMs (IN) Amount of time in milliseconds to wait for the MDM lock.
 *
 * @return TRUE if user is authenticated.
 */
UBOOL8 cmsDal_authenticate(HttpLoginType *authLevel, NetworkAccessMode accessMode,
                         const char *username,
                         const char *passwd);

/** Set current primary and secondary DNS server names for ppp ip extension only.
 *
 * @param wanIp         (IN)  ppp wan ip
 * @param dnsServerList (IN)  dns server list
 *
 * @return CmsRet enum.
 *
 */
CmsRet cmsDal_setDhcpRelayConfig(const char *wanIp,  const char *dnsServerList);


/** Find the Layer 2 bridging entry object with the given bridge key.
 *
 * @param bridgeKey (IN) bridge key.
 * @param iidStack (OUT) If found, this iidStack will be filled out with
 *                       the instance info for the found obj.
 * @param bridgeObj (OUT) If found, this pointer will be set to the
 *                        L2BridgingEntryObject.  Caller is responsible for
 *                        freeing this object.
 *
 * @return CmsRet enum.
 */
CmsRet dalPMap_getBridgeByKey(UINT32 bridgeKey,
                              InstanceIdStack *iidStack,
                              L2BridgingEntryObject **bridgeObj);


/** Get the bridge key of the specified bridge.
 *
 * @param bridgeName (IN) name of the bridge
 * @param bridgeKey (OUT) If the bridge is found, the bridge key of the bridge
 *                        is set in this variable.
 * @return CmsRet enum.
 */
CmsRet dalPMap_getBridgeKey(const char *bridgeName, UINT32 *bridgeKey);


/** Add a new LAN side interface group (layer 2 bridge) and enable it.
 *
 * @param bridgeName (IN) The name of the interface group (not Linux ifName)
 *
 * @return CmsRet enum.
 */
CmsRet dalPMap_addBridge(IntfGrpBridgeMode mode, const char *bridgeName);
CmsRet dalPMap_addBridge_igd(IntfGrpBridgeMode mode, const char *bridgeName);
CmsRet dalBridge_addBridge_dev2(IntfGrpBridgeMode mode, const char *bridgeName);

#if defined(SUPPORT_DM_LEGACY98)
#define dalPMap_addBridge(m, g)  dalPMap_addBridge_igd((m), (g))
#elif defined(SUPPORT_DM_HYBRID)
#define dalPMap_addBridge(m, g)  dalPMap_addBridge_igd((m), (g))
#elif defined(SUPPORT_DM_PURE181)
#define dalPMap_addBridge(m, g)  dalBridge_addBridge_dev2((m), (g))
#elif defined(SUPPORT_DM_DETECT)
#define dalPMap_addBridge(m, g)  (cmsMdm_isDataModelDevice2() ? \
                               dalBridge_addBridge_dev2((m), (g)) : \
                               dalPMap_addBridge_igd((m), (g)))
#endif



/** Add bridges for BEEP project by interface group interface.
 *
 * @return CmsRet enum.
 */
CmsRet dalPMap_addBeepBridge(void);



/** Delete bridges for BEEP project by interface group interface.
 *
 */
void dalPMap_deleteBeepBridge(void);



/** Remove the Layer2BridgingEntry object in the MDM.
 *
 * @param bridgeName (IN) The name of the interface group (not Linux ifName)
 *
 */
void dalPMap_deleteBridge(const char *bridgeName);
void dalPMap_deleteBridge_igd(const char *bridgeName);
void dalBridge_deleteBridge_dev2(const char *bridgeName);

#if defined(SUPPORT_DM_LEGACY98)
#define dalPMap_deleteBridge(g)  dalPMap_deleteBridge_igd((g))
#elif defined(SUPPORT_DM_HYBRID)
#define dalPMap_deleteBridge(g)  dalPMap_deleteBridge_igd((g))
#elif defined(SUPPORT_DM_PURE181)
#define dalPMap_deleteBridge(g)  dalBridge_deleteBridge_dev2((g))
#elif defined(SUPPORT_DM_DETECT)
#define dalPMap_deleteBridge(g)  (cmsMdm_isDataModelDevice2() ? \
                               dalBridge_deleteBridge_dev2((g)) : \
                               dalPMap_deleteBridge_igd((g)))
#endif




/** Find the Layer 2 bridging filter interface object with the given name.
 *
 * @param ifName (IN) name of the filter interface.
 * @param iidStack (OUT) If found, this iidStack will be filled out with
 *                       the instance info for the found obj.
 * @param filterIntfObj (OUT) If found, this pointer will be set to the
 *                L2BridgingFilterObject.  The caller is responsible for
 *                freeing this object.
 *
 * @return CmsRet enum.
 */
CmsRet dalPMap_getFilterIntf(const char *ifName, InstanceIdStack *iidStack, L2BridgingFilterObject **filterObj);


/** Find the Layer 2 bridging filter object associated with the specified
 *  bridge and uses a DHCP Vendor Id filter.
 *
 * We assume there is only 1 such filter associated with each bridge.
 *
 * @param ifName (IN) name of the filter interface.
 * @param iidStack (OUT) If found, this iidStack will be filled out with
 *                       the instance info for the found obj.
 *
 * @param filterIntfObj (OUT) If found, this pointer will be set to the
 *                L2BridgingFilterObject.  The caller is responsible for
 *                freeing this object.
 *
 * @return CmsRet enum.
 */
CmsRet dalPMap_getFilterDhcpVendorIdByBridgeName(const char *ifName, InstanceIdStack *iidStack, L2BridgingFilterObject **filterObj);




/** Associate the DHCP vendor id string with the interface group.
 *
 *  TR98 notes: Create a new Layer 2 bridging filter object that is associated
 *  with the specified bridge and use the specified aggregate DHCP Vendor id
 *  string.
 *
 * @param bridgeName (IN) Name of the interface group.
 * @param aggregateString (IN) Aggregate DHCP Vendor Id string as returned by
 *                             cmsUtl_getAggregateStringFromDhcpVendorIds().
 *
 * @return CmsRet enum.
 */
CmsRet dalPMap_addFilterDhcpVendorId(const char *bridgeName, const char *aggregateString);
CmsRet dalPMap_addFilterDhcpVendorId_igd(const char *bridgeName, const char *aggregateString);
CmsRet dalBridge_addFilterDhcpVendorId_dev2(const char *bridgeName, const char *aggregateString);

#if defined(SUPPORT_DM_LEGACY98)
#define dalPMap_addFilterDhcpVendorId(g, s)  dalPMap_addFilterDhcpVendorId_igd((g), (s))
#elif defined(SUPPORT_DM_HYBRID)
#define dalPMap_addFilterDhcpVendorId(g, s)  dalPMap_addFilterDhcpVendorId_igd((g), (s))
#elif defined(SUPPORT_DM_PURE181)
#define dalPMap_addFilterDhcpVendorId(g, s)  dalBridge_addFilterDhcpVendorId_dev2((g), (s))
#elif defined(SUPPORT_DM_DETECT)
#define dalPMap_addFilterDhcpVendorId(g, s)  (cmsMdm_isDataModelDevice2() ? \
                           dalBridge_addFilterDhcpVendorId_dev2((g), (s)) : \
                           dalPMap_addFilterDhcpVendorId_igd((g), (s)))
#endif



/** Delete the association of the DHCP vendorId with the interface group.
 *
 *  TR98 notes: delete the Layer 2 bridging filter object that is associated
 *  with the specified bridge and uses DHCP Vendor id string.  We assume there
 *  is only 1 filter per bridge that uses the DHCP Vendor Id string, so by
 *  by supplying only the bridge name, we should be able to find the filter.
 *
 *
 * @param bridgeName (IN) Name of the interface group.
 *
 */
void dalPMap_deleteFilterDhcpVendorId(const char *bridgeName);
void dalPMap_deleteFilterDhcpVendorId_igd(const char *bridgeName);
void dalBridge_deleteFilterDhcpVendorId_dev2(const char *bridgeName);

#if defined(SUPPORT_DM_LEGACY98)
#define dalPMap_deleteFilterDhcpVendorId(g)  dalPMap_deleteFilterDhcpVendorId_igd((g))
#elif defined(SUPPORT_DM_HYBRID)
#define dalPMap_deleteFilterDhcpVendorId(g)  dalPMap_deleteFilterDhcpVendorId_igd((g))
#elif defined(SUPPORT_DM_PURE181)
#define dalPMap_deleteFilterDhcpVendorId(g)  dalBridge_deleteFilterDhcpVendorId_dev2((g))
#elif defined(SUPPORT_DM_DETECT)
#define dalPMap_deleteFilterDhcpVendorId(g)  (cmsMdm_isDataModelDevice2() ? \
                           dalBridge_deleteFilterDhcpVendorId_dev2((g)) : \
                           dalPMap_deleteFilterDhcpVendorId_igd((g)))
#endif



/** Associate a (WAN or LAN) Linux interface with the specified interface
 *  group name.
 *  In TR98, set the Layer2BridgingFilter->filterBridgeReference field.
 *
 * @param ifName (IN) The Linux interface name of the interface to associate
 *                    with the specified bridge.
 *
 * @param grpName (IN) The user friendly bridge name, aka interface group name.
 *                     As a special case, if grpName is NULL, then this
 *                     interface is disassociated from all bridges.
 *
 *
 * @return CmsRet enum.
 */
CmsRet dalPMap_assocFilterIntfToBridge(const char *ifName, const char *grpName);
CmsRet dalPMap_assocFilterIntfToBridge_igd(const char *ifName, const char *grpName);
CmsRet dalBridge_assocFilterIntfToBridge_dev2(const char *ifName, const char *grpName);

#if defined(SUPPORT_DM_LEGACY98)
#define dalPMap_assocFilterIntfToBridge(i, g)  dalPMap_assocFilterIntfToBridge_igd((i), (g))
#elif defined(SUPPORT_DM_HYBRID)
#define dalPMap_assocFilterIntfToBridge(i, g)  dalPMap_assocFilterIntfToBridge_igd((i), (g))
#elif defined(SUPPORT_DM_PURE181)
#define dalPMap_assocFilterIntfToBridge(i, g)  dalBridge_assocFilterIntfToBridge_dev2((i), (g))
#elif defined(SUPPORT_DM_DETECT)
#define dalPMap_assocFilterIntfToBridge(i, g)  (cmsMdm_isDataModelDevice2() ? \
                           dalBridge_assocFilterIntfToBridge_dev2((i), (g)) : \
                           dalPMap_assocFilterIntfToBridge_igd((i), (g)))
#endif



/** Disassociate all filter interfaces from the specified bridge and associate them
 * with the Default bridge.
 *
 * @param bridgeName (IN) The bridge name (cannot be the default bridge).
 *
 * @return CmsRet enum.
 */
CmsRet dalPMap_disassocAllFilterIntfFromBridge(const char *bridgeName);
CmsRet dalPMap_disassocAllFilterIntfFromBridge_igd(const char *bridgeName);
CmsRet dalBridge_disassocAllFilterIntfFromBridge_dev2(const char *bridgeName);

#if defined(SUPPORT_DM_LEGACY98)
#define dalPMap_disassocAllFilterIntfFromBridge(g)  dalPMap_disassocAllFilterIntfFromBridge_igd((g))
#elif defined(SUPPORT_DM_HYBRID)
#define dalPMap_disassocAllFilterIntfFromBridge(g)  dalPMap_disassocAllFilterIntfFromBridge_igd((g))
#elif defined(SUPPORT_DM_PURE181)
#define dalPMap_disassocAllFilterIntfFromBridge(g)  dalBridge_disassocAllFilterIntfFromBridge_dev2((g))
#elif defined(SUPPORT_DM_DETECT)
#define dalPMap_disassocAllFilterIntfFromBridge(g)  (cmsMdm_isDataModelDevice2() ? \
                           dalBridge_disassocAllFilterIntfFromBridge_dev2((g)) : \
                           dalPMap_disassocAllFilterIntfFromBridge_igd((g)))
#endif




/** Find the Layer 2 bridging available interface object with the specified
 *  InterfaceReference parameter.
 *
 * @param availInterfaceReference (IN) Value of the InterfaceReference
 *                       parameter in the AvailableInterface object.
 * @param iidStack (OUT) If found, this iidStack will be filled out with
 *                       the instance info for the found obj.
 * @param filterIntfObj (OUT) If found, this pointer will be set to the
 *                L2BridgingIntfObject.  The caller is responsible for freeing
 *                the object.
 *
 * @return CmsRet enum.
 */
CmsRet dalPMap_getAvailableInterfaceByRef(const char *availInterfaceReference, InstanceIdStack *iidStack, L2BridgingIntfObject **availIntfObj);


/** Find the Layer 2 bridging available interface object with the specified
 *  AvailableInterfaceKey.
 *
 * @param availInterfaceKey (IN) Value of the AvailableInterfaceKey
 *                       parameter in the AvailableInterface object.
 * @param iidStack (OUT) If found, this iidStack will be filled out with
 *                       the instance info for the found obj.
 * @param availIntfObj (OUT) If found, this pointer will be set to the
 *                L2BridgingIntfObject.  The caller is responsible for freeing
 *                the object.
 *
 * @return CmsRet enum.
 */
CmsRet dalPMap_getAvailableInterfaceByKey(UINT32 availableInterfaceKey, InstanceIdStack *iidStack, L2BridgingIntfObject **availIntfObj);


/** Given a WAN IfName, get the full param name in the format required
 * by the InterfaceReference parameter name of the AvailableInterface object.
 *
 * @param ifName (IN) name of the available interface.
 * @param availInterfaceReference (OUT) This must be a buffer of 256 bytes long.
 *                    On success, this buffer will be filled with the full param name.
 *
 * @return CmsRet enum.
 */
CmsRet dalPMap_wanIfNameToAvailableInterfaceReference(const char *ifName, char *availInterfaceReference);


/** Given a LAN IfName, get the full param name in the format required
 * by the InterfaceReference parameter name of the AvailableInterface object.
 *
 * @param ifName (IN) name of the available interface.
 * @param availInterfaceReference (OUT) This must be a buffer of 256 bytes long.
 *                    On success, this buffer will be filled with the full param name.
 *
 * @return CmsRet enum.
 */
CmsRet dalPMap_lanIfNameToAvailableInterfaceReference(const char *ifName, char *availInterfaceReference);


/** Convert a LAN IfName to the full param name in the format required
 * by the InterfaceReference parameter name of the AvailableInterface object.
 *
 * @param ifName (IN) name of the available interface.
 * @param availInterfaceReference (OUT) This must be a buffer of 256 bytes long.
 *                    On success, this buffer will be filled with the full param name.
 *
 * @return CmsRet enum.
 */
CmsRet dalPMap_lanIfNameToAvailableInterfaceReference(const char *lanIfName, char *availableInterfaceReference);


/** Convert the InterfaceReference parameter in the available interface object
 *  to the Linux interface name.
 *
 * @param availableInterfaceReference (IN) The interface reference parameter value
 *               in the available interface object.
 * @param ifName (OUT) This buffer must be at least 32 bytes long.  On successful
 *               return, this will contain the Linux ifName associated with
 *               the availableInterfaceReference.
 *
 * @return CmsRet enum
 */
CmsRet dalPMap_availableInterfaceReferenceToIfName(const char *availableInterfaceReference, char *ifName);


/** Enable or disable virtual ports on the ethernet switch.
 *
 * @param cfgStatus (IN) TRUE or FALSE for enable or disable.
 *
 */
void dalEsw_enableVirtualPorts(UBOOL8 cfgStatus);


/** Read eterhnet switch info and put it into glbWebVar
 *
 * @param (OUT) pointer to the webvar struct to fill out.
 */
CmsRet dalEsw_getEthernetSwitchInfo(WEB_NTWK_VAR *webVar);




/** Set the upnp enable flag if it has been changed.
 * @param enableUpnp    (IN)  The new upnp enable value
 * @param *outNeedSave  (OUT) If a new value is set, need to set to TRUE for saving to flash.
 *
 * @return CmsRet enum.
 */
CmsRet dalUpnp_config(UBOOL8 enableUpnp, UBOOL8 *outNeedSave);
/** add a dynamic dns entry.
 *
 * @param fullyQualifiedDomainName (IN) fully qualified domain name
 * @param userName       (IN) user name
 * @param password       (IN) user password
 * @param interface      (IN) interface
 * @param providerName   (IN) provider name
 *
 * @return CmsRet enum.
 */

CmsRet dalDDns_addEntry(const char *fullyQualifiedDomainName, const char *userName, const char *password, const char *interface, unsigned short int providerName);



/** delete a dynamic dns entry.
 *
 * @param fullyQualifiedDomainName (IN) fully qualified domain name
 *
 * @return CmsRet enum.
 */
CmsRet dalDDns_deleteEntry(const char *fullyQualifiedDomainName);




/** update an automatic 6rd tunnel entry.
 *
 * @param prefix               (IN) 6rd prefix
 * @param brAddr             (IN) border relay address
 * @param ifName             (IN) associated WAN interface name
 * @param ipv4MaskLen     (IN) IPv4 mask length
 * @param ipv6PrefixLen    (IN) 6rd prefix length
 *
 **/
void dalTunnel_update6rdObject( const char *prefix, const char *brAddr, const char *ifName,
                                SINT32 ipv4MaskLen, SINT32 ipv6PrefixLen );
void dalTunnel_update6rdObject_igd( const char *prefix, const char *brAddr, const char *ifName,
                                SINT32 ipv4MaskLen, SINT32 ipv6PrefixLen );
void dalTunnel_update6rdObject_dev2( const char *prefix, const char *brAddr, const char *ifName,
                                SINT32 ipv4MaskLen, SINT32 ipv6PrefixLen );


#if defined(SUPPORT_DM_LEGACY98)
#define dalTunnel_update6rdObject(n, m, w, l, d)  dalTunnel_update6rdObject_igd((n), (m), (w), (l), (d))
#elif defined(SUPPORT_DM_HYBRID)
#define dalTunnel_update6rdObject(n, m, w, l, d)  dalTunnel_update6rdObject_dev2((n), (m), (w), (l), (d))
#elif defined(SUPPORT_DM_PURE181)
#define dalTunnel_update6rdObject(n, m, w, l, d)  dalTunnel_update6rdObject_dev2((n), (m), (w), (l), (d))
#elif defined(SUPPORT_DM_DETECT)
#define dalTunnel_update6rdObject(n, m, w, l, d)  (cmsMdm_isDataModelDevice2() ? \
                                    dalTunnel_update6rdObject_dev2((n), (m), (w), (l), (d)) : \
                                    dalTunnel_update6rdObject_dev2((n), (m), (w), (l), (d)))
#endif


#ifdef SUPPORT_IPV6

/** update an automatic DS-Lite tunnel entry.
 *
 * @param aftr               (IN) aftr name
 * @param ifName             (IN) associated WAN interface name
 *
 **/
void dalTunnel_updateDSLiteObject( const char *aftr, const char *ifName );


/** add an IPv6 encapsulated in IPv4 tunnel entry.
 *
 * @param tunnelName    (IN) tunnel name
 * @param mechanism    (IN) type of the tunnel
 * @param wanIntf          (IN) associated WAN interface
 * @param lanIntf           (IN) associated LAN interface
 * @param dynamic        (IN) static or dynamic tunnel
 * @param ipv6rdPrefix   (IN) 6rd prefix (with length)
 * @param ipv4MaskLen  (IN) IPv4 mask length
 * @param brAddr           (IN) border relay server address
 *
 * @return CmsRet enum.
 */
CmsRet dal6in4Tunnel_add(const char* tunnelName, const char *mechanism, const char *wanIntf, const char *lanIntf,
	                                           UBOOL8 dynamic, const char *ipv6rdPrefix, SINT8 ipv4MaskLen, const char *brAddr);
CmsRet dal6in4Tunnel_add_igd(const char* tunnelName, const char *mechanism, const char *wanIntf, const char *lanIntf,
	                     UBOOL8 dynamic, const char *ipv6rdPrefix, SINT8 ipv4MaskLen, const char *brAddr);
CmsRet dal6in4Tunnel_add_dev2(const char* tunnelName, const char *mechanism, const char *wanIntf, const char *lanIntf,
	                     UBOOL8 dynamic, const char *ipv6rdPrefix, SINT8 ipv4MaskLen, const char *brAddr);

#if defined(SUPPORT_DM_LEGACY98)
#define dal6in4Tunnel_add(n, m, w, l, d, p, s, a)  dal6in4Tunnel_add_igd((n), (m), (w), (l), (d), (p), (s), (a))
#elif defined(SUPPORT_DM_HYBRID)
#define dal6in4Tunnel_add(n, m, w, l, d, p, s, a)  dal6in4Tunnel_add_dev2((n), (m), (w), (l), (d), (p), (s), (a))
#elif defined(SUPPORT_DM_PURE181)
#define dal6in4Tunnel_add(n, m, w, l, d, p, s, a)  dal6in4Tunnel_add_dev2((n), (m), (w), (l), (d), (p), (s), (a))
#elif defined(SUPPORT_DM_DETECT)
#define dal6in4Tunnel_add(n, m, w, l, d, p, s, a)  (cmsMdm_isDataModelDevice2() ? \
                                    dal6in4Tunnel_add_dev2((n), (m), (w), (l), (d), (p), (s), (a)) : \
                                    dal6in4Tunnel_add_dev2((n), (m), (w), (l), (d), (p), (s), (a)))
#endif


/** delete a tunnel entry.
 *
 * @param tunnelName   (IN) tunnel name
 * @param mode            (IN) tunnel mode
 *
 * @return CmsRet enum.
 **/
CmsRet dalTunnel_delete(const char* tunnelName, const char * mode);
CmsRet dalTunnel_delete_igd(const char* tunnelName, const char * mode);
CmsRet dalTunnel_delete_dev2(const char* tunnelName, const char * mode);

#if defined(SUPPORT_DM_LEGACY98)
#define dalTunnel_delete(n, m)  dalTunnel_delete_igd((n), (m))
#elif defined(SUPPORT_DM_HYBRID)
#define dalTunnel_delete(n, m)  dalTunnel_delete_dev2((n), (m))
#elif defined(SUPPORT_DM_PURE181)
#define dalTunnel_delete(n, m)  dalTunnel_delete_dev2((n), (m))
#elif defined(SUPPORT_DM_DETECT)
#define dalTunnel_delete(n, m)  (cmsMdm_isDataModelDevice2() ? \
                                          dalTunnel_delete_dev2((n), (m)) : \
                                          dalTunnel_delete_dev2((n), (m)))
#endif


/** add an IPv4 encapsulated in IPv6 tunnel entry.
 *
 * @param tunnelName    (IN) tunnel name
 * @param mechanism    (IN) type of the tunnel
 * @param wanIntf          (IN) associated WAN interface
 * @param lanIntf           (IN) associated LAN interface
 * @param dynamic        (IN) static or dynamic tunnel
 * @param remoteIp       (IN) remote IPv6 address
 *
 * @return CmsRet enum.
 */
CmsRet dal4in6Tunnel_add(const char* tunnelName, const char *mechanism, const char *wanIntf, const char *lanIntf,
	                                           UBOOL8 dynamic, const char *remoteIp);
CmsRet dal4in6Tunnel_add_igd(const char* tunnelName, const char *mechanism, const char *wanIntf, const char *lanIntf,
	                     UBOOL8 dynamic, const char *remoteIp);
CmsRet dal4in6Tunnel_add_dev2(const char* tunnelName, const char *mechanism, const char *wanIntf, const char *lanIntf,
	                     UBOOL8 dynamic, const char *remoteIp);

#if defined(SUPPORT_DM_LEGACY98)
#define dal4in6Tunnel_add(n, m, w, l, d, i)  dal4in6Tunnel_add_igd((n), (m), (w), (l), (d), (i))
#elif defined(SUPPORT_DM_HYBRID)
#define dal4in6Tunnel_add(n, m, w, l, d, i)  dal4in6Tunnel_add_dev2((n), (m), (w), (l), (d), (i))
#elif defined(SUPPORT_DM_PURE181)
#define dal4in6Tunnel_add(n, m, w, l, d, i)  dal4in6Tunnel_add_dev2((n), (m), (w), (l), (d), (i))
#elif defined(SUPPORT_DM_DETECT)
#define dal4in6Tunnel_add(n, m, w, l, d, i)  (cmsMdm_isDataModelDevice2() ? \
                                    dal4in6Tunnel_add_dev2((n), (m), (w), (l), (d), (i)) : \
                                    dal4in6Tunnel_add_dev2((n), (m), (w), (l), (d), (i)))
#endif


/** add MAP-T/MAP-E Interface.
 *
 * @param mechanism      (IN) transport mode
 * @param wanIntf        (IN) associated WAN interface
 * @param lanIntf        (IN) associated LAN interface
 * @param dynamic        (IN) static or dynamic tunnel
 * @param BRPrefix       (IN) Border relay prefix
 * @param ipv6Prefix     (IN) BMR IPv6 prefix
 * @param ipv4Prefix     (IN) BMR IPv4 prefix
 * @param psidOffset     (IN) BMR PSID offset
 * @param psidLen        (IN) BMR PSID length
 * @param psid           (IN) BMR PSID value
 *
 * @return CmsRet enum.
 */
CmsRet dalMap_add_dev2(const char* mechanism, const char *wanIntf, const char *lanIntf,
                       UBOOL8 dynamic, const char *BRPrefix,
                       const char *ipv6Prefix, const char *ipv4Prefix,
                       UINT32 psidOffset, UINT32 psidLen, UINT32 psid);


/** delete a MAP-T/MAP-E entry.
 *
 * @param wanIntf       (IN) associated WAN
 * @param mode          (IN) transport mode
 *
 * @return CmsRet enum.
 **/
CmsRet dalMap_delete_dev2(const char* wanIntf, const char* mode);


/** add a delegated address entry and only called by ssk.
 *
 * @param srvName   (IN) tunnel name or interface name
 * @param ipv6str      (IN) IPv6 address to be assinged
 * @param lanIntf       (IN) LAN interface to be assigned
 * @param mode        (IN) tunnel mode or wan mode
 *
 * @return CmsRet enum.
 */
CmsRet dalWan_addDelegatedAddrEntry(const char *srvName, const char *ipv6str,
                                                                            const char *lanIntf, const char * mode);


/** add an IPv6 static route entry.
 *
 * @param addr    (IN) destination IPv6 address with subnet prefix length in CIDR notation
 * @param gateway (IN) gateway IPv6 address of this static route
 * @param wanif   (IN) default interface of this static route
 * @param metric  (IN) hop number to the destination
 *
 * @return CmsRet enum.
 */
CmsRet dalStaticRoute6_addEntry(const char* addr, const char *gateway, const char *wanif, const char *metric);
CmsRet dalStaticRoute6_addEntry_igd(const char* addr, const char *gateway, const char *wanif, const char *metric);
CmsRet dalStaticRoute6_addEntry_dev2(const char* addr, const char *gateway, const char *wanif, const char *metric);

#if defined(SUPPORT_DM_LEGACY98)
#define dalStaticRoute6_addEntry(a, g, i, m)  dalStaticRoute6_addEntry_igd((a), (g), (i), (m))
#elif defined(SUPPORT_DM_HYBRID)
#define dalStaticRoute6_addEntry(a, g, i, m)  dalStaticRoute6_addEntry_dev2((a), (g), (i), (m))
#elif defined(SUPPORT_DM_PURE181)
#define dalStaticRoute6_addEntry(a, g, i, m)  dalStaticRoute6_addEntry_dev2((a), (g), (i), (m))
#elif defined(SUPPORT_DM_DETECT)
#define dalStaticRoute6_addEntry(a, g, i, m)  (cmsMdm_isDataModelDevice2() ? \
                                        dalStaticRoute6_addEntry_dev2((a), (g), (i), (m)) : \
                                        dalStaticRoute6_addEntry_dev2((a), (g), (i), (m)))
#endif


/** delete an IPv6 static route entry.
 *
 * @param addr      (IN) destination IPv6 address with subnet prefix length in CIDR notation
 *
 * @return CmsRet enum.
 **/
CmsRet dalStaticRoute6_deleteEntry(const char* addr);
CmsRet dalStaticRoute6_deleteEntry_igd(const char* addr);
CmsRet dalStaticRoute6_deleteEntry_dev2(const char* addr);

#if defined(SUPPORT_DM_LEGACY98)
#define dalStaticRoute6_deleteEntry(a)  dalStaticRoute6_deleteEntry_igd((a))
#elif defined(SUPPORT_DM_HYBRID)
#define dalStaticRoute6_deleteEntry(a)  dalStaticRoute6_deleteEntry_dev2((a))
#elif defined(SUPPORT_DM_PURE181)
#define dalStaticRoute6_deleteEntry(a)  dalStaticRoute6_deleteEntry_dev2((a))
#elif defined(SUPPORT_DM_DETECT)
#define dalStaticRoute6_deleteEntry(a)  (cmsMdm_isDataModelDevice2() ? \
                                    dalStaticRoute6_deleteEntry_dev2((a)) : \
                                    dalStaticRoute6_deleteEntry_dev2((a)))
#endif




/** Set the IPv6L3Forwarding object for system ipv6 default gateway interface.
 *
 * @param gwIfc      (IN) The gateway interface name.
 *
 * @return CmsRet enum.
 */
CmsRet cmsDal_setSysDfltGw6Ifc(char *gw6Ifc);
CmsRet cmsDal_setSysDfltGw6Ifc_igd(char *gw6Ifc);
CmsRet cmsDal_setSysDfltGw6Ifc_dev2(char *gw6Ifc);

#if defined(SUPPORT_DM_LEGACY98)
#define cmsDal_setSysDfltGw6Ifc(g)  cmsDal_setSysDfltGw6Ifc_igd((g))
#elif defined(SUPPORT_DM_HYBRID)
#define cmsDal_setSysDfltGw6Ifc(g)  cmsDal_setSysDfltGw6Ifc_dev2((g))
#elif defined(SUPPORT_DM_PURE181)
#define cmsDal_setSysDfltGw6Ifc(g)  cmsDal_setSysDfltGw6Ifc_dev2((g))
#elif defined(SUPPORT_DM_DETECT)
#define cmsDal_setSysDfltGw6Ifc(g)  (cmsMdm_isDataModelDevice2() ? \
                                    cmsDal_setSysDfltGw6Ifc_dev2((g)) : \
                                    cmsDal_setSysDfltGw6Ifc_dev2((g)))
#endif


CmsRet dalLan_setLan6Cfg_igd(const WEB_NTWK_VAR *glbWebVar);
CmsRet dalLan_setLan6Cfg_dev2(const WEB_NTWK_VAR *glbWebVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalLan_setLan6Cfg(w)  dalLan_setLan6Cfg_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define dalLan_setLan6Cfg(w)  dalLan_setLan6Cfg_dev2((w))
#elif defined(SUPPORT_DM_PURE181)
#define dalLan_setLan6Cfg(w)  dalLan_setLan6Cfg_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define dalLan_setLan6Cfg(w)  (cmsMdm_isDataModelDevice2() ? \
                               dalLan_setLan6Cfg_dev2((w)) : \
                               dalLan_setLan6Cfg_dev2((w)))
#endif

#endif   /* SUPPORT_IPV6 */

/** Set the ripd after WEB configuration change
 *
 * @param pIfcName    (IN) WAN interface name
 * @param pRipVer     (IN) rip version (0= Off, 1 = V1, 2=V2, 3=V1V2)
 * @param pOperation  (IN) operation mode (0= ACTIVE, 1 = PASSIVE)
 * @param pEnabled    (IN) 0 = Off, 1 = ON
 *
 */

#ifdef SUPPORT_RIP
CmsRet dalRip_setRipEntry(char *pIfcName, char *pRipVer, char *pOperation, char *pEnabled);
CmsRet dalRip_setRipEntry_igd(char *pIfcName, char *pRipVer, char *pOperation, char *pEnabled);
CmsRet dalRip_setRipEntry_dev2(char *pIfcName, char *pRipVer, char *pOperation, char *pEnabled);

#if defined(SUPPORT_DM_LEGACY98)
#define dalRip_setRipEntry(pIfcName, pRipVer, pOperation, pEnabled)  dalRip_setRipEntry_igd((pIfcName),(pRipVer),(pOperation),(pEnabled))
#elif defined(SUPPORT_DM_HYBRID)
#define dalRip_setRipEntry(pIfcName, pRipVer, pOperation, pEnabled)  dalRip_setRipEntry_igd((pIfcName),(pRipVer),(pOperation),(pEnabled))
#elif defined(SUPPORT_DM_PURE181)
#define dalRip_setRipEntry(pIfcName, pRipVer, pOperation, pEnabled)  dalRip_setRipEntry_dev2((pIfcName),(pRipVer),(pOperation),(pEnabled))
#elif defined(SUPPORT_DM_DETECT)
#define dalRip_setRipEntry(pIfcName, pRipVer, pOperation, pEnabled)  (cmsMdm_isDataModelDevice2() ? \
                                    dalRip_setRipEntry_dev2((pIfcName),(pRipVer),(pOperation),(pEnabled)) : \
                                    dalRip_setRipEntry_igd((pIfcName),(pRipVer),(pOperation),(pEnabled)))
#endif

CmsRet dalRip_addRipInterfaceSetting_dev2(const char *ipIntfFullPath);
CmsRet dalRip_deleteRipInterfaceSetting_dev2(const char *ipIntfFullPath);
#endif


#ifdef SUPPORT_IPP
/** Get the IppCfgObj for print server info.
 *
 * @param ippEnabled   (OUT) print server enable boolean.
 * @param ippMake      (OUT) the printer make.
 * @param ippName      (OUT) the printer name.
 *
 * @return CmsRet enum.
 */
CmsRet cmsDal_getPrintServerInfo(char *ippEnabled, char *ippMake, char *ippName);
#endif

/** Get the DmsCfgObj for digital media server info.
 *
 * @param dmsEnabled   (OUT) digital media server enable boolean.
 * @param dmsMediaPath (OUT) location of media files.
 *
 * @return CmsRet enum.
 */
CmsRet cmsDal_getDigitalMediaServerInfo(char *dmsEnabled, char *dmsMediaPath, char *dmsBrName);




/** Get available L2 Eth interface
 *
 * @param ifList  (OUT) the list of available interface.
 *
 * @return CmsRet enum.
 */
CmsRet cmsDal_getAvailableL2EthIntf(NameList **ifList);
CmsRet cmsDal_getAvailableL2EthIntf_igd(NameList **ifList);
CmsRet dalEth_getAvailableL2EthIntf_dev2(NameList **ifList);

#if defined(SUPPORT_DM_LEGACY98)
#define cmsDal_getAvailableL2EthIntf(a)  cmsDal_getAvailableL2EthIntf_igd((a))
#elif defined(SUPPORT_DM_HYBRID)
#define cmsDal_getAvailableL2EthIntf(a)  cmsDal_getAvailableL2EthIntf_igd((a))
#elif defined(SUPPORT_DM_PURE181)
#define cmsDal_getAvailableL2EthIntf(a)  dalEth_getAvailableL2EthIntf_dev2((a))
#elif defined(SUPPORT_DM_DETECT)
#define cmsDal_getAvailableL2EthIntf(a)  (cmsMdm_isDataModelDevice2() ? \
                                    dalEth_getAvailableL2EthIntf_dev2((a)) : \
                                    cmsDal_getAvailableL2EthIntf_igd((a)))
#endif


/** Get available GMAC Eth interface
 *
 * @param gMACEthIntf  (OUT) the list of available GMAC Eth interfaces.
 *
 * @return CmsRet enum.
 */
CmsRet  cmsDal_getGMACEthIntf(char *gMACEthIntf);
CmsRet  cmsDal_getGMACEthIntf_igd(char *gMACEthIntf);
CmsRet  dalEth_getGMACEthIntf_dev2(char *gMACEthIntf);

#if defined(SUPPORT_DM_LEGACY98)
#define cmsDal_getGMACEthIntf(a)  cmsDal_getGMACEthIntf_igd((a))
#elif defined(SUPPORT_DM_HYBRID)
#define cmsDal_getGMACEthIntf(a)  cmsDal_getGMACEthIntf_igd((a))
#elif defined(SUPPORT_DM_PURE181)
#define cmsDal_getGMACEthIntf(a)  dalEth_getGMACEthIntf_dev2((a))
#elif defined(SUPPORT_DM_DETECT)
#define cmsDal_getGMACEthIntf(a)  (cmsMdm_isDataModelDevice2() ? \
                                    dalEth_getGMACEthIntf_dev2((a)) : \
                                    cmsDal_getGMACEthIntf_igd((a)))
#endif



/** Get WAN Only Eth interface
 *
 * @param gWanOnlyEthIntf  (OUT) the list of available WAN Only Eth interfaces.
 *
 * @return CmsRet enum.
 */
CmsRet  cmsDal_getWANOnlyEthIntf(char *gWanOnlyEthIntf);
CmsRet  cmsDal_getWANOnlyEthIntf_igd(char *gWanOnlyEthIntf);
CmsRet  cmsDal_getWANOnlyEthIntf_dev2(char *gWanOnlyEthIntf);

#if defined(SUPPORT_DM_LEGACY98)
#define cmsDal_getWANOnlyEthIntf(a)  cmsDal_getWANOnlyEthIntf_igd((a))
#elif defined(SUPPORT_DM_HYBRID)
#define cmsDal_getWANOnlyEthIntf(a)  cmsDal_getWANOnlyEthIntf_igd((a))
#elif defined(SUPPORT_DM_PURE181)
#define cmsDal_getWANOnlyEthIntf(a)  cmsDal_getWANOnlyEthIntf_dev2((a))
#elif defined(SUPPORT_DM_DETECT)
#define cmsDal_getWANOnlyEthIntf(a)  (cmsMdm_isDataModelDevice2() ? \
                                    cmsDal_getWANOnlyEthIntf_dev2((a)) : \
                                    cmsDal_getWANOnlyEthIntf_igd((a)))
#endif


CmsRet cmsDal_getConfiguredWanEthIntf(char *gConfiguredWanEthIntf);
CmsRet cmsDal_getConfiguredWanEthIntf_igd(char *gConfiguredWanEthIntf);
CmsRet cmsDal_getConfiguredWanEthIntf_dev2(char *gConfiguredWanEthIntf);

#if defined(SUPPORT_DM_LEGACY98)
#define cmsDal_getConfiguredWanEthIntf(a)  cmsDal_getConfiguredWanEthIntf_igd((a))
#elif defined(SUPPORT_DM_HYBRID)
#define cmsDal_getConfiguredWanEthIntf(a)  cmsDal_getConfiguredWanEthIntf_igd((a))
#elif defined(SUPPORT_DM_PURE181)
#define cmsDal_getConfiguredWanEthIntf(a)  cmsDal_getConfiguredWanEthIntf_dev2((a))
#elif defined(SUPPORT_DM_DETECT)
#define cmsDal_getConfiguredWanEthIntf(a)  (cmsMdm_isDataModelDevice2() ? \
                                    cmsDal_getConfiguredWanEthIntf_dev2((a)) : \
                                    cmsDal_getConfiguredWanEthIntf_igd((a)))
#endif


/** if device has ETH WAN
 *
 * @param none
 *
 * @return UBOOL8 indicating whether ETH wan be set.
 */
UBOOL8 dalEth_isEthWanMode();


/** if device has DSL wan service
 *
 * @param none
 *
 * @return UBOOL8 indicating whether DSL wan service be set.
 */
UBOOL8 dalEth_hasDslWanService();


/* Get WanEthIntfObject by ETH wan interface name
 *
 * @param ifname     (IN)  The LAN interface name.
 * @param iidStack (OUT) iidStack of the ethIntfCfg object found.
 * @param ethIntfCfg (OUT) if not null, this will contain a pointer to the found
 *                         ethIntfCfg object.  Caller is responsible for calling
 *                         cmsObj_free() on this object.
 *
 * @return UBOOL8 indicating whether the desired ethIntfCfg object was found.
 */
UBOOL8 dalEth_getEthIntfByIfName(char *ifName, InstanceIdStack *iidStack, WanEthIntfObject **ethIntfCfg);




/** Make the specified eth interface a WAN interface
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 *
 * @return CmsRet enum.
 */
CmsRet dalEth_addEthInterface(const WEB_NTWK_VAR *webVar);

CmsRet dalEth_addEthInterface_igd(const WEB_NTWK_VAR *webVar);

CmsRet dalEth_addEthInterface_dev2(const WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalEth_addEthInterface(w)  dalEth_addEthInterface_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define dalEth_addEthInterface(w)  dalEth_addEthInterface_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define dalEth_addEthInterface(w)  dalEth_addEthInterface_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define dalEth_addEthInterface(w)  (cmsMdm_isDataModelDevice2() ? \
                                    dalEth_addEthInterface_dev2((w)) : \
                                    dalEth_addEthInterface_igd((w)))
#endif




/** Make the specified eth interface a LAN interface (instead of WAN intf).
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 *
 * @return CmsRet enum.
 */
CmsRet dalEth_deleteEthInterface(const WEB_NTWK_VAR *webVar);

CmsRet dalEth_deleteEthInterface_igd(const WEB_NTWK_VAR *webVar);

CmsRet dalEth_deleteEthInterface_dev2(const WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalEth_deleteEthInterface(w)  dalEth_deleteEthInterface_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define dalEth_deleteEthInterface(w)  dalEth_deleteEthInterface_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define dalEth_deleteEthInterface(w)  dalEth_deleteEthInterface_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define dalEth_deleteEthInterface(w)  (cmsMdm_isDataModelDevice2() ? \
                                       dalEth_deleteEthInterface_dev2((w)) : \
                                       dalEth_deleteEthInterface_igd((w)))
#endif




/** Helper function for writing to the NETWORK_CONTROL object, for moving
 * interfaces to/from WAN/LAN.  Exactly one of moveToLanSide or
 * moveToWanSide must be set to TRUE.  This function can be used in
 * TR98 and TR181 code.
 *
 * @param l2IntfName (IN)  The layer 2 interface name to be moved
 * @param moveToLan  (IN) move from WAN to LAN
 * @param moveToWan  (IN) move from LAN to WAN
 *
 * @return CmsRet
 */
CmsRet cmsDal_moveIntfLanWan(const char *l2IntfName,
                             UBOOL8 moveToLan, UBOOL8 moveToWan);


/** Helper function needed when moving interfaces between WAN and LAN.
 *  We need to release the lock and let ssk update the intfStack and
 *  its internal data structures at certain points during the move operation.
 *  Use this function with care!!  Once you release the lock, the MDM might
 *  have changed.
 */
void cmsDal_releaseAndRelock(void);




/** Add a new name to the NameList.  This function will allocate a new
 *  NameList struct and append it to the end of the list.
 *
 * @param name  (IN)     the name to add
 * @param nlist (IN/OUT) pointer to the NameList ptr.
 *
 * @return On success, returns pointer to the newly created NameList element,
 *         which has been hooked into the end of the list.  On failure,
 *         return NULL
 */
NameList *cmsDal_addNameToNameList(const char *name, NameList **nlist);


/** Free memory allocated for name list.
 *
 * @param nl  (IN) the name list.
 */
void cmsDal_freeNameList(NameList *nl);




/** Get a list of LAN Moca interfaces which can be moved to WAN side.
 *
 * @param ifList  (OUT) the list of available interface.  Caller is
 *                      responsible for freeing the list by calling
 *                      cmsDal_freeNameList.
 *
 * @return CmsRet enum.
 */
CmsRet cmsDal_getAvailableL2MocaIntf(NameList **ifList);
CmsRet cmsDal_getAvailableL2MocaIntf_igd(NameList **ifList);
CmsRet cmsDal_getAvailableL2MocaIntf_dev2(NameList **ifList);

#if defined(SUPPORT_DM_LEGACY98)
#define cmsDal_getAvailableL2MocaIntf(v)  cmsDal_getAvailableL2MocaIntf_igd((v))
#elif defined(SUPPORT_DM_HYBRID)
#define cmsDal_getAvailableL2MocaIntf(v)  cmsDal_getAvailableL2MocaIntf_igd((v))
#elif defined(SUPPORT_DM_PURE181)
#define cmsDal_getAvailableL2MocaIntf(v)  cmsDal_getAvailableL2MocaIntf_dev2((v))
#elif defined(SUPPORT_DM_DETECT)
#define cmsDal_getAvailableL2MocaIntf(v)  (cmsMdm_isDataModelDevice2() ? \
                                cmsDal_getAvailableL2MocaIntf_dev2((v)) : \
                                cmsDal_getAvailableL2MocaIntf_igd((v)))
#endif




/** Get available L2 Gpon interfaces
 *
 * @param ifList  (OUT) the list of available interface.
 *
 * @return CmsRet enum.
 */
CmsRet cmsDal_getAvailableL2GponIntf_igd(NameList **ifList);
CmsRet cmsDal_getAvailableL2GponIntf_dev2(NameList **ifList);

#if defined(SUPPORT_DM_LEGACY98)
#define cmsDal_getAvailableL2GponIntf(v)  cmsDal_getAvailableL2GponIntf_igd((v))
#elif defined(SUPPORT_DM_HYBRID)
#define cmsDal_getAvailableL2GponIntf(v)  cmsDal_getAvailableL2GponIntf_igd((v))
#elif defined(SUPPORT_DM_PURE181)
#define cmsDal_getAvailableL2GponIntf(v)  cmsDal_getAvailableL2GponIntf_dev2((v))
#elif defined(SUPPORT_DM_DETECT)
#define cmsDal_getAvailableL2GponIntf(v)  (cmsMdm_isDataModelDevice2() ? \
                                cmsDal_getAvailableL2GponIntf_dev2((v)) : \
                                cmsDal_getAvailableL2GponIntf_igd((v)))
#endif


/** Get available L2 Epon interfaces
 *
 * @param ifList  (OUT) the list of available interface.
 *
 * @return CmsRet enum.
 */
CmsRet cmsDal_getAvailableL2EponIntf_igd(NameList **ifList);
CmsRet cmsDal_getAvailableL2EponIntf_dev2(NameList **ifList);

#if defined(SUPPORT_DM_LEGACY98)
#define cmsDal_getAvailableL2EponIntf(v)  cmsDal_getAvailableL2EponIntf_igd((v))
#elif defined(SUPPORT_DM_HYBRID)
#define cmsDal_getAvailableL2EponIntf(v)  cmsDal_getAvailableL2EponIntf_igd((v))
#elif defined(SUPPORT_DM_PURE181)
#define cmsDal_getAvailableL2EponIntf(v)  cmsDal_getAvailableL2EponIntf_dev2((v))
#elif defined(SUPPORT_DM_DETECT)
#define cmsDal_getAvailableL2EponIntf(v)  (cmsMdm_isDataModelDevice2() ? \
                                cmsDal_getAvailableL2EponIntf_dev2((v)) : \
                                cmsDal_getAvailableL2EponIntf_igd((v)))
#endif


/** Get current HomePlug configuration parameters.
 *
 * @param webData (IN/OUT) pointer to global web data structure
 *
 * @return CmsRet enum.
 */
CmsRet dalHomeplug_getCurrentCfg(WEB_NTWK_VAR* webVar);

CmsRet dalIeee1905_getCurrentCfg(WEB_NTWK_VAR* webVar);


#ifdef SUPPORT_CERT

/*
 * A lot of the defs here really do not belong this high level
 * common header file.  They should get moved closer to where they
 * are used.  Often, these defs are not even shared.
 */

#define CERT_LOCAL_MAX_ENTRY      4
#define CERT_CA_MAX_ENTRY         4

#define CERT_BUFF_MAX_LEN         300
#define CERT_NAME_LEN             64
#define CERT_TYPE_SIGNING_REQ     "request"
#define CERT_TYPE_SIGNED          "signed"
#define CERT_TYPE_CA              "ca"

#define CERT_KEY_MAX_LEN          3500

#define CERT_TYPE_LEN             8

#define CERT_KEY_SIZE             1024

#define CERT_LOCAL 1
#define CERT_CA 2


#define CERT_SUBJECT_CN       0
#define CERT_SUBJECT_O        1
#define CERT_SUBJECT_ST       2
#define CERT_SUBJECT_C        3

typedef struct {
   char certName[CERT_NAME_LEN];
   char commonName[CERT_NAME_LEN];
   char country[CERT_BUFF_MAX_LEN];
   char state[CERT_NAME_LEN];
   char organization[CERT_NAME_LEN];
} CERT_ADD_INFO, *PCERT_ADD_INFO;

/** Get the number of existed certificates that match the given certificate type
 *
 * @param type       (IN) certificate type

 * @return the number of existed certificates that match the given certificate type
 */
UINT32 dalCert_getNumberOfExistedCert(SINT32 type);

/** Find the existed certificate that matches the given name and type
 *
 * @param name     (IN) certificate name
 * @param type       (IN) certificate type

 * @return TRUE if certificate is found, otherwise return FALSE
  */
UBOOL8 dalCert_findCert(char *name, SINT32 type);

/* Copy the certificate object with the input data entry
 *
 * @param obj               (OUT) the certCfg object that needs to be filled
 * @param certCfg         (IN) the input certificate
 *
 * @return CmsRet enum.
 */
CmsRet dalCert_copyCert(CertificateCfgObject *obj, CertificateCfgObject *certCfg);

/** add a certificate entry.
 *
 * @param certCfg         (IN) the input certificate
 *
 * @return CmsRet enum.
 */
CmsRet dalCert_addCert(CertificateCfgObject *certCfg);

/** retrieve a certificate object that matches the given name and type
 *
 * @param name           (IN) the certificate name
 * @param type             (IN) the certificate type
 * @param certCfg         (OUT) the certificate object
 *
 * @return CmsRet enum.
 */
CmsRet dalCert_getCert(char *name, SINT32 type, CertificateCfgObject *certCfg);

/** store a certificate object that matches the given name and type
 *
 * @param name           (IN) the certificate name
 * @param type             (IN) the certificate type
 * @param certCfg         (IN) the certificate entry
 *
 * @return CmsRet enum.
 */
CmsRet dalCert_setCert(char *name, SINT32 type, CertificateCfgObject *certCfg);

/** remove a certificate object that matches the given name and type
 *
 * @param cert         (IN) the certificate name
 * @param cert         (IN) the certificate type
 *
 * @return CmsRet enum.
 */
CmsRet dalCert_delCert(char *name, SINT32 type);

/** reset all reference count to 0 for signed or signed request certificates
 * but not for CA certificates
 *
 * @return CmsRet enum.
 */
CmsRet dalCert_resetRefCount(void);

/** Increment/decrement reference count of the given local certificate.
 * This function always returns CMSRET_OBJECT_NOT_FOUND for ca certificates
 * or if the given name isn't on the local certificate list.
 *
 * @param name           (IN) the certificate name
 *
 * @return CmsRet enum.
 */
CmsRet dalCert_incRefCount(char *name);
CmsRet dalCert_decRefCount(char *name);

/** Set symbolic links for CA certificates (needed by ssl)
 *
 * @param cert         (IN) the input certificate
 *
 */
void rutCert_setCACertLinks(void);

/** create Openssl configuration file.
 *
 * @param pAddInfo       (IN) the certificate information
 *
 */
void rutCert_createCertConf(PCERT_ADD_INFO pAddInfo);

/** create certificate request
 *
 * @param pAddInfo       (IN) the certificate information
 * @param certCfg         (IN) the certificate object
 *
 * @return CmsRet enum.
 */
CmsRet rutCert_createCertReq(PCERT_ADD_INFO pAddInfo, CertificateCfgObject *certCfg);

/** create signed certificate or CA certificate
 *
 * @param certCfg         (IN) the certificate object
 *
 * @return CmsRet enum.
 */
CmsRet rutCert_createCertFile(const CertificateCfgObject *certCfg);

/** remove signed certificate or CA certificate
 *
 * @param certCfg         (IN) the certificate object
 *
 * @return CmsRet enum.
 */
CmsRet rutCert_removeCertFile(const CertificateCfgObject *certCfg);

/** compare two files
 *
 * @param fn1         (IN) the name of the first file
 * @param fn2         (IN) the name of the second file
 *
 * @return TRUE if the content of the first file is the same with the second file, otherwise return FALSE.
 */
UBOOL8 rutCert_compareFile(char *fn1, char *fn2);

/** Verify issued certificate against request
 *
 * @param certCfg         (IN) the certificate object
 *
 * @return CmsRet enum.
 */
CmsRet rutCert_verifyCertReq(CertificateCfgObject *certCfg);

/** Get subject of certificate
 *
 * @param certCfg         (IN) the certificate object
 *
 * @return CmsRet enum.
 */
CmsRet rutCert_retrieveSubject(CertificateCfgObject *certCfg);

/** Get serial number, signature algorithm, issuer,
 * not before, not after, and subject of certificate
 *
 * @param certCfg         (IN) the certificate object
 * @param numberOfArgs    (IN) number of data to retrieve
 * @param serialNumber    (OUT) the certificate serial number
 * @param signature       (OUT) the certificate signature algorithm
 * @param issuer          (OUT) the certificate issuer
 * @param notBefore       (OUT) the certificate not before
 * @param notAfter        (OUT) the certificate not after
 * @param subject         (OUT) the certificate subject

 * @return CmsRet enum.
 */
CmsRet rutCert_retrieveInfo
   (const CertificateCfgObject *certCfg,
    UINT32 numberOfArgs,
    char *serialNumber,
    char *signature,
    char *issuer,
    char *notBefore,
    char *notAfter,
    char *subject);

/** Process and verify imported certificate and private key
 *
 * @param certCfg         (IN) the certificate object
 *
 * @return CmsRet enum.
 */
CmsRet rutCert_processImportedCert(CertificateCfgObject *certCfg);



#endif   /* SUPPORT_CERT */


#define PMIRROR_DISABLED                           0
#define PMIRROR_ENABLED                            1
#define PMIRROR_DIR_IN                             0
#define PMIRROR_DIR_OUT                            1


/** Configure port mirroring
 *
 * @param lst         (IN) the string contains port mirroring information for configuration
 *
 * @return CmsRet enum.
 */
CmsRet dalPMirror_configPortMirrors(char *lst);

void dalPMirror_fillPMirrorEntry(const char *l2IfName, char *pMirrorList, SINT32 pMListSize);

/** Get port mirroring for engdebug.cmd web page.
 *
 * @param lst         (IN) the string contains port mirroring information for configuration
 *
 * @return CmsRet enum.
 */
void dalPMirror_getPMirrorList(char *lst);

void dalPMirror_getPMirrorList_igd(char *lst);

void dalPMirror_getPMirrorList_dev2(char *lst);

#if defined(SUPPORT_DM_LEGACY98)
#define dalPMirror_getPMirrorList(w)        dalPMirror_getPMirrorList_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define dalPMirror_getPMirrorList(w)        dalPMirror_getPMirrorList_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define dalPMirror_getPMirrorList(w)        dalPMirror_getPMirrorList_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define dalPMirror_getPMirrorList(w)        (cmsMdm_isDataModelDevice2() ? \
                                             dalPMirror_getPMirrorList_dev2((w)) : \
                                             dalPMirror_getPMirrorList_igd((w)))

#endif

#ifdef DMP_X_ITU_ORG_GPON_1
/** Get GPON port mirroring for engdebug.cmd web page.
 *
 * @param lst         (IN) the string contains port mirroring information for configuration
 *
 * @return CmsRet enum.
 */
void dalPMirror_getGponPMirrorList(char *lst);
#endif




#if defined(DMP_TIME_1) || defined(DMP_DEVICE2_TIME_1)

/** Call appropriate cmsObj_set function to set the timeSever config info.
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 *
 * @return CmsRet enum.
 */
CmsRet cmsDal_setNtpCfg(const WEB_NTWK_VAR *webVar);

CmsRet cmsDal_setNtpCfg_igd(const WEB_NTWK_VAR *webVar);

CmsRet cmsDal_setNtpCfg_dev2(const WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define cmsDal_setNtpCfg(w)        cmsDal_setNtpCfg_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define cmsDal_setNtpCfg(w)        cmsDal_setNtpCfg_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define cmsDal_setNtpCfg(w)        cmsDal_setNtpCfg_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define cmsDal_setNtpCfg(w)        (cmsMdm_isDataModelDevice2() ? \
                                    cmsDal_setNtpCfg_dev2((w)) : \
                                    cmsDal_setNtpCfg_igd((w)))
#endif


/** Get the current timeServer config info from MDM and fill in appropriate
 *  fields in webVar.
 *
 * @param (OUT) pointer to WEB_NTWK_VAR
 *
 * @return CmsRet enum.
 */
void cmsDal_getNtpCfg(WEB_NTWK_VAR *webVar);

void cmsDal_getNtpCfg_igd(WEB_NTWK_VAR *webVar);

void cmsDal_getNtpCfg_dev2(WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define cmsDal_getNtpCfg(w)        cmsDal_getNtpCfg_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define cmsDal_getNtpCfg(w)        cmsDal_getNtpCfg_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define cmsDal_getNtpCfg(w)        cmsDal_getNtpCfg_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define cmsDal_getNtpCfg(w)        (cmsMdm_isDataModelDevice2() ? \
                                    cmsDal_getNtpCfg_dev2((w)) : \
                                    cmsDal_getNtpCfg_igd((w)))
#endif

#endif /* DMP_TIME_1 or DMP_DEVICE2_TIME_1 */




/** Fill in webVar for the configured non DMZ LAN info
 * @param nonDmzIpAddress  (OUT) Non DMZ ip address.
 * @param nonDmzIpMask     (OUT) Non DMZ ip address mask.
 */
void dalNtwk_fillInNonDMZIpAndMask(char *nonDmzIpAddress, char *nonDmzIpMask);

/** call rut_isAdvancedDmzEnabled to return the configuration flag
 * for if advanced DMZ is enabled in the system
 *
 */
UBOOL8 dalWan_isAdvancedDmzEnabled(void);

/** call rutWan_isPPPIpExtension to return if the system is configured as
 * ppp ip extension
 *
 */
UBOOL8 dalWan_isPPPIpExtension(void);



/** call rutPMap_isWanUsedForIntfGroup to find out
 * if this WAN interface is used in the interface group
 */
UBOOL8 dalPMap_isWanUsedForIntfGroup(const char *ifName);

/** Start/Stop Ping diagnostics
 *
 * @param pingParms         (IN) all ping parameters (i.e. repetition, host address, ...)
 *
 * @return CmsRet enum.
 */
CmsRet dalDiag_startStopPing(IPPingDiagObject *pingParms);

/** Get Ping diagnostics result -- get the most recent ping test result
 *
 * @param pingParms         (IN) structure to hold ping results.   Ping's state is a string,
 *                          caller needs to free the state field when done reading the result.
 *
 * @return CmsRet enum.
 */
CmsRet dalDiag_getPingResult(void *pingResult);
CmsRet dalDiag_getPingResult_igd(void *pingResult);
CmsRet dalDiag_getPingResult_dev2(void *pingResult);
#if defined(SUPPORT_DM_LEGACY98)
#define dalDiag_getPingResult(w)       dalDiag_getPingResult_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define dalDiag_getPingResult(w)       dalDiag_getPingResult_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define dalDiag_getPingResult(w)       dalDiag_getPingResult_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define dalDiag_getPingResult(w)       (cmsMdm_isDataModelDevice2() ? \
                                        dalDiag_getPingResult_dev2((w)) :  \
                                        dalDiag_getPingResult_igd((w)))
#endif


/** Start and wait for OAM Loopback test result
 *
 * @param type         (IN) type of loopback (BCM_DIAG_OAM_SEGMENT,BCM_DIAG_OAM_END2END,
 *                          BCM_DIAG_OAM_F4_SEGMENT,BCM_DIAG_OAM_F4_END2END)
 * @param addr         (IN) PORT/VPI/VCI to send loopback on
 * @param repetition   (IN) number of loopback cell to send.
 * @param timeout      (IN) timeout value in second waiting for loopback response
 *
 *
 * @return CmsRet enum (OUT) Success/Pass if CMS_SUCCESS; otherwise test fails.
 */
CmsRet dalDiag_doOamLoopback(int type, PPORT_VPI_VCI_TUPLE addr, UINT32 repetition, UINT32 timeout);
CmsRet dalDiag_doOamLoopback_igd(int type, PPORT_VPI_VCI_TUPLE addr, UINT32 repetition, UINT32 timeout);
CmsRet dalDiag_doOamLoopback_dev2(int type, PPORT_VPI_VCI_TUPLE addr, UINT32 repetition, UINT32 timeout);
#if defined(SUPPORT_DM_LEGACY98)
#define dalDiag_doOamLoopback(a,b,c,d)       dalDiag_doOamLoopback_igd((a),(b),(c),(d))
#elif defined(SUPPORT_DM_HYBRID)
#define dalDiag_doOamLoopback(a,b,c,d)       dalDiag_doOamLoopback_igd((a),(b),(c),(d))
#elif defined(SUPPORT_DM_PURE181)
#define dalDiag_doOamLoopback(a,b,c,d)       dalDiag_doOamLoopback_dev2((a),(b),(c),(d))
#elif defined(SUPPORT_DM_DETECT)
#define dalDiag_doOamLoopback(a,b,c,d)       (cmsMdm_isDataModelDevice2() ? \
                                              dalDiag_doOamLoopback_dev2((a),(b),(c),(d)) : \
                                              dalDiag_doOamLoopback_igd((a),(b),(c),(d)))
#endif

#if defined(DMP_IPPING_1)
CmsRet dalDiag_startStopPing(IPPingDiagObject *pingParms);
CmsRet dalDiag_startStopPing_igd(IPPingDiagObject *pingParms);
#endif
#if defined(DMP_DEVICE2_IPPING_1)
CmsRet dalDiag_startStopPing_dev2(Dev2IpPingDiagObject *pingParms);
#endif
#if defined(SUPPORT_DM_LEGACY98)
#define dalDiag_startStopPing(w)       dalDiag_startStopPing_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define dalDiag_startStopPing(w)       dalDiag_startStopPing_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define dalDiag_startStopPing(w)       dalDiag_startStopPing_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define dalDiag_startStopPing(w)       (cmsMdm_isDataModelDevice2() ? \
                                        dalDiag_startStopPing_dev2((w)) :  \
                                        dalDiag_startStopPing_igd((w)))
#endif




#ifdef SUPPORT_DSL

/** Find out if ATM or PTM link's statistics should be displayed
 *
 * @param (IN) isAtm boolean pointer
 * @param (OUT)isAtm is TRUE if ATM statistics should be displayed;
 *                   otherwise, display PTM.  The default is ATM.
 * @return CmsRet enum.
 */
CmsRet dalDsl_displayXtmStatsType_dev2(UBOOL8 *isAtm);

/** Delete the dslLinkConfig object
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 *
 * @return CmsRet enum.
 */
CmsRet dalDsl_deleteAtmInterface(WEB_NTWK_VAR *webVar);
CmsRet dalDsl_deleteAtmInterface_igd(WEB_NTWK_VAR *webVar);
CmsRet dalDsl_deleteAtmInterface_dev2(WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalDsl_deleteAtmInterface(w)  dalDsl_deleteAtmInterface_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define dalDsl_deleteAtmInterface(w)  dalDsl_deleteAtmInterface_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define dalDsl_deleteAtmInterface(w)  dalDsl_deleteAtmInterface_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define dalDsl_deleteAtmInterface(w)  (cmsMdm_isDataModelDevice2() ? \
                                      dalDsl_deleteAtmInterface_dev2((w)) : \
                                      dalDsl_deleteAtmInterface_igd((w)))
#endif



/** Delete ATM Interface that has
 * match with portId, atmVpi, atmVci in the webVar WITHOUT interface name
 *
 * @param webVar (IN) Pointer to the WEB_NTWK_VAR.
 *
 * @return CMSRET_SUCCESS or errors.
 */
CmsRet dalDsl_deleteAtmInterfaceWithoutIfName(WEB_NTWK_VAR *webVar);
CmsRet dalDsl_deleteAtmInterfaceWithoutIfName_igd(WEB_NTWK_VAR *webVar);
CmsRet dalDsl_deleteAtmInterfaceWithoutIfName_dev2(WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalDsl_deleteAtmInterfaceWithoutIfName(i)     dalDsl_deleteAtmInterfaceWithoutIfName_igd((i))
#elif defined(SUPPORT_DM_HYBRID)
#define dalDsl_deleteAtmInterfaceWithoutIfName(i)      dalDsl_deleteAtmInterfaceWithoutIfName_igd((i))
#elif defined(SUPPORT_DM_PURE181)
#define dalDsl_deleteAtmInterfaceWithoutIfName(i)      dalDsl_deleteAtmInterfaceWithoutIfName_dev2((i))
#elif defined(SUPPORT_DM_DETECT)
#define dalDsl_deleteAtmInterfaceWithoutIfName(i)   (cmsMdm_isDataModelDevice2() ? \
                                       dalDsl_deleteAtmInterfaceWithoutIfName_dev2((i)) : \
                                       dalDsl_deleteAtmInterfaceWithoutIfName_igd((i)))
#endif



/** Add the dslLinkConfig object
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 *
 * @return CmsRet enum.
 */
CmsRet dalDsl_addAtmInterface(const WEB_NTWK_VAR *webVar);
CmsRet dalDsl_addAtmInterface_igd(const WEB_NTWK_VAR *webVar);
CmsRet dalDsl_addAtmInterface_dev2(const WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalDsl_addAtmInterface(w)    dalDsl_addAtmInterface_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define dalDsl_addAtmInterface(w)    dalDsl_addAtmInterface_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define dalDsl_addAtmInterface(w)    dalDsl_addAtmInterface_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define dalDsl_addAtmInterface(w)    (cmsMdm_isDataModelDevice2() ? \
                                      dalDsl_addAtmInterface_dev2((w)) : \
                                      dalDsl_addAtmInterface_igd((w)))
#endif


/** get the default atmLinkConfig info to webVar
 *
 * @param (OUT) pointer to WEB_NTWK_VAR
 *
 */
void getDefaultAtmLinkCfg(WEB_NTWK_VAR *webVar);
void getDefaultAtmLinkCfg_igd(WEB_NTWK_VAR *webVar);
void getDefaultAtmLinkCfg_dev2(WEB_NTWK_VAR *webVar);
#if defined(SUPPORT_DM_LEGACY98)
#define getDefaultAtmLinkCfg(w)       getDefaultAtmLinkCfg_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define getDefaultAtmLinkCfg(w)       getDefaultAtmLinkCfg_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define getDefaultAtmLinkCfg(w)       getDefaultAtmLinkCfg_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define getDefaultAtmLinkCfg(w)       (cmsMdm_isDataModelDevice2() ? \
                                       getDefaultAtmLinkCfg_dev2((w)) : \
                                       getDefaultAtmLinkCfg_igd((w)))
#endif


/** get the default ptmLinkConfig info to webVar
 *
 * @param (OUT) pointer to WEB_NTWK_VAR
 *
 */
void getDefaultPtmLinkCfg(WEB_NTWK_VAR *webVar);
void getDefaultPtmLinkCfg_igd(WEB_NTWK_VAR *webVar);
void getDefaultPtmLinkCfg_dev2(WEB_NTWK_VAR *webVar);
#if defined(SUPPORT_DM_LEGACY98)
#define getDefaultPtmLinkCfg(w)       getDefaultPtmLinkCfg_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define getDefaultPtmLinkCfg(w)       getDefaultPtmLinkCfg_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define getDefaultPtmLinkCfg(w)       getDefaultPtmLinkCfg_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define getDefaultPtmLinkCfg(w)       (cmsMdm_isDataModelDevice2() ? \
                                       getDefaultPtmLinkCfg_dev2((w)) : \
                                       getDefaultPtmLinkCfg_igd((w)))
#endif


/** get the default dslLinkConfig info to webVar
 *
 * @param (OUT) pointer to WEB_NTWK_VAR
 *
 */
void getDefaultWanDslLinkCfg(WEB_NTWK_VAR *webVar);
void getDefaultWanDslLinkCfg_igd(WEB_NTWK_VAR *webVar);
void getDefaultWanDslLinkCfg_dev2(WEB_NTWK_VAR *webVar);
#if defined(SUPPORT_DM_LEGACY98)
#define getDefaultWanDslLinkCfg(w)       getDefaultWanDslLinkCfg_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define getDefaultWanDslLinkCfg(w)       getDefaultWanDslLinkCfg_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define getDefaultWanDslLinkCfg(w)       getDefaultWanDslLinkCfg_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define getDefaultWanDslLinkCfg(w)       (cmsMdm_isDataModelDevice2() ? \
                                          getDefaultWanDslLinkCfg_dev2((w)) : \
                                          getDefaultWanDslLinkCfg_igd((w)))
#endif


/** find the dslLinkCfg object with the given destAddr.
 *
 * This function assumes there is only one WANDevice.
 *
 * @param portId   (IN) ATM interface port id
 * @param destAddr (IN) vpi/vci of the DslLinkCfg to find.
 * @param iidStack (OUT) iidStack of the DslLinkCfg object found.
 * @param dslLinkCfg (OUT) if not null, this will contain a pointer to the found
 *                         dslLinkCfg object.  Caller is responsible for calling
 *                         cmsObj_free() on this object.
 *
 * @return UBOOL8 indicating whether the desired DslLinkCfg object was found.
 */
UBOOL8 getDslLinkCfg(UINT32 portId, const char *destAddr, InstanceIdStack *iidStack, WanDslLinkCfgObject **dslLinkCfg);


/** get WanDev for ATM/PTM iidStack.  Uses rut function.
 *
 * This function assumes both ATM and PTM WANDevice is initialized by mdm_init
 *
 * @param isAtm   (IN) TRUE is for DSL ATM WanDev, FALSE is PTM WanDev
 * @param iidStack (OUT) iidStack of the WanDev object for ATM/PTM found.
 *
 * @return UBOOL8 indicating whether the desired WanDev for ATM or PTM object was found.
 */
UBOOL8 dalDsl_getDslWanDevIidStack(UBOOL8 isAtm, InstanceIdStack *wanDevIid);




#ifdef SUPPORT_PTM

/** Add the WanPtmLinkCfgObject object
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 *
 * @return CmsRet enum.
 */
CmsRet dalDsl_addPtmInterface(const WEB_NTWK_VAR *webVar);
CmsRet dalDsl_addPtmInterface_igd(const WEB_NTWK_VAR *webVar);
CmsRet dalDsl_addPtmInterface_dev2(const WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalDsl_addPtmInterface(w)    dalDsl_addPtmInterface_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define dalDsl_addPtmInterface(w)    dalDsl_addPtmInterface_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define dalDsl_addPtmInterface(w)    dalDsl_addPtmInterface_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define dalDsl_addPtmInterface(w)    (cmsMdm_isDataModelDevice2() ? \
                                      dalDsl_addPtmInterface_dev2((w)) : \
                                      dalDsl_addPtmInterface_igd((w)))
#endif



/** find the ptmLinkCfg object with the given portId, priority and return if ptmWanDev
 * exist or not and if existed, the ptmWanDev IidStack and ptmLinkCfg IidStack and object.
 *
 * This function assumes there is only one WANDevice.
 *
 * @param portId        (IN) PTM interface port id
 * @param priorityNorm  (IN) normal priority
 * @param priorityHigh  (IN) High priority
 * @param ptmIid        (OUT) iidStack of the ptm Wan Dev object found.
 *  @parm  pmCfgObj     (OUT) ptmLinkCfg object
 *
 * @return UBOOL8 indicating whether the desired DslLinkCfg object was found.
 */
UBOOL8 dalDsl_getPtmLinkCfg(UINT32 portId,
                            UINT32 priorityNorm,
                            UINT32 priorityHigh,
                            InstanceIdStack *ptmIid,
                            WanPtmLinkCfgObject **ptmCfgObj);

/** Delete the WanPtmLinkCfgObject object
 *
 * @Param (IN) pointer to WEB_NTWK_VAR
 *
 * @return CmsRet enum.
 */
 CmsRet dalDsl_deletePtmInterface(WEB_NTWK_VAR *webVar);
 CmsRet dalDsl_deletePtmInterface_igd(WEB_NTWK_VAR *webVar);
 CmsRet dalDsl_deletePtmInterface_dev2(WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalDsl_deletePtmInterface(w)  dalDsl_deletePtmInterface_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define dalDsl_deletePtmInterface(w)  dalDsl_deletePtmInterface_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define dalDsl_deletePtmInterface(w)  dalDsl_deletePtmInterface_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define dalDsl_deletePtmInterface(w)  (cmsMdm_isDataModelDevice2() ? \
                                      dalDsl_deletePtmInterface_dev2((w)) : \
                                      dalDsl_deletePtmInterface_igd((w)))
#endif



/** Delete PTM Interface without Interface Name but has X_BROADCOM_COM_PTMPortId,
 * X_BROADCOM_COM_PTMPriorityHigh, X_BROADCOM_COM_PTMPriorityLow
 * match with portId, ptmPriorityHigh, ptmPriorityNormal in the webVar.
 *
 * @param webVar (IN) Pointer to the WEB_NTWK_VAR.
 *
 * @return CMSRET_SUCCESS or errors.
 */
 CmsRet dalDsl_deletePtmInterfaceWithoutIfName(WEB_NTWK_VAR *webVar);
 CmsRet dalDsl_deletePtmInterfaceWithoutIfName_igd(WEB_NTWK_VAR *webVar);
 CmsRet dalDsl_deletePtmInterfaceWithoutIfName_dev2(WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalDsl_deletePtmInterfaceWithoutIfName(w)  dalDsl_deletePtmInterfaceWithoutIfName_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define dalDsl_deletePtmInterfaceWithoutIfName(w)  dalDsl_deletePtmInterfaceWithoutIfName_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define dalDsl_deletePtmInterfaceWithoutIfName(w)  dalDsl_deletePtmInterfaceWithoutIfName_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define dalDsl_deletePtmInterfaceWithoutIfName(w)  (cmsMdm_isDataModelDevice2() ? \
                                      dalDsl_deletePtmInterfaceWithoutIfName_dev2((w)) : \
                                      dalDsl_deletePtmInterfaceWithoutIfName_igd((w)))
#endif


#endif   /* SUPPORT_PTM */

#endif   /* SUPPORT_DSL */




/** Get fresh from MDM the default gateway list and dns info
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 *
 */
void cmsDal_getDefaultGatewayAndDnsCfg(WEB_NTWK_VAR *webVar);




#ifdef DMP_X_ITU_ORG_GPON_1
/** Call appropriate cmsObj_set function to set the OMCI system related info.
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 *
 * @return CmsRet enum.
 */
CmsRet cmsDal_setOmciSystem(const WEB_NTWK_VAR *webVar);

/** Load MDM OMCI system object info into webVar.
 *
 * @param webVar (OUT) webvar variable to be filled in.
 */
void cmsDal_getOmciSystem(WEB_NTWK_VAR *webVar);
#endif  /* DMP_X_ITU_ORG_GPON_1 */

#ifdef DMP_X_BROADCOM_COM_ETHERNETOAM_1
void dalEthOam_getAllCfg(WEB_NTWK_VAR *pWebVar);
#endif

/** get the next available Connection Id against this layer 2 wan interface name
 * @param wanL2IfName (IN) layer 2 ifName.
 * @param outConId (OUT) connection Id
 *
 * @return CmsRet enum.
 */
CmsRet cmsDal_getAvailableConIdForMSC(const char *wanL2IfName, SINT32 *outConId);

/** get the number of used xtm transmit queues.
 * @param wanType      (IN) wan protocol type
 * @param usedQueues   (OUT) the number of used xtm transmit queues.
 *
 * @return CmsRet enum.
 */
CmsRet cmsDal_getNumUsedQueues(WanLinkType wanType, UINT32 *usedQueues);


/** get the number of unused xtm transmit queues.
 * @param wanType      (IN) wan protocol type
 * @param unusedQueues (OUT) the number of unused xtm transmit queues.
 *
 * @return CmsRet enum.
 */
CmsRet cmsDal_getNumUnusedQueues(WanLinkType wanType, UINT32 *unusedQueues);



/** Return true if the specified interface name exists as a WAN interface
 *  name in the MDM (does not check in the kernel for the existence of
 *  this interface).
 *
 * @param (IN) WAN interface name
 *
 * @return TRUE if WAN interface exists in the MDM.
 */
UBOOL8 dalWan_isValidWanInterface(const char *ifName);

UBOOL8 dalWan_isValidWanInterface_igd(const char *ifName);

UBOOL8 dalWan_isValidWanInterface_dev2(const char *ifName);

#if defined(SUPPORT_DM_LEGACY98)
#define dalWan_isValidWanInterface(i)  dalWan_isValidWanInterface_igd((i))
#elif defined(SUPPORT_DM_HYBRID)
#define dalWan_isValidWanInterface(i)  dalWan_isValidWanInterface_igd((i))
#elif defined(SUPPORT_DM_PURE181)
#define dalWan_isValidWanInterface(i)  dalWan_isValidWanInterface_dev2((i))
#elif defined(SUPPORT_DM_DETECT)
#define dalWan_isValidWanInterface(i)  (cmsMdm_isDataModelDevice2() ? \
                               dalWan_isValidWanInterface_dev2((i)) : \
                               dalWan_isValidWanInterface_igd((i)))
#endif



/** Validate the given ifNameList (for DNSIfNames and defaultGatewayIfNames)
 *
 * @param ifNameList (IN) input ifname list
 * @param maxCount   (IN) Max number of ifNames supported/allowed
 * @param returnNewList (OUT) On successful return, will contain the validated
 *             ifNameList with spaces stripped out and all ifNames validated.
 *             Caller is responsible for freeing this buffer.
 *
 * @return CmsRet enum
 */
CmsRet dalWan_validateIfNameList(const char *ifNameList,
                                 UINT32 maxCount,
                                 char **returnNewList);



/**  Set the enable flag in ip/ppp WAN connection object
 *
 * @param ifName     (IN) WAN Conn interface name
 * @param isEnabled (IN) Boolean if TRUE, Wan Connection object is enabled.
 * @return CmsRet
 */
CmsRet dalWan_enableDisableWanConnObj(const char *ifName, UBOOL8 isEnabled);


#ifdef DMP_X_BROADCOM_COM_PWRMNGT_1
/** configure power mangement conf. params
 * @param pWebVar (IN) WEB_NTWK_VAR type
 *
 * @return CmsRet enum.
 */
CmsRet dalPowerManagement(const PWEB_NTWK_VAR pWebVar);

/** get power mangement conf. params
 * @param pWebVar (IN/OUT) WEB_NTWK_VAR type
 *
 * @return CmsRet enum.
 */
CmsRet dalGetPowerManagement(WEB_NTWK_VAR *pWebVar);
#endif /* aka SUPPORT_PWRMNGT */

#ifdef SUPPORT_BMU
CmsRet dalGetBmu(WEB_NTWK_VAR *pWebVar, void *msgHandleArg);
#endif

#ifdef DMP_X_BROADCOM_COM_STANDBY_1
/** configure standby conf. params
 * @param pWebVar (IN) WEB_NTWK_VAR type
 *
 * @return CmsRet enum.
 */
CmsRet dalStandby(const PWEB_NTWK_VAR pWebVar);
CmsRet dalStandbyDemo(void *msgHandle);

/** get standby conf. params
 * @param pWebVar (IN/OUT) WEB_NTWK_VAR type
 *
 * @return CmsRet enum.
 */
CmsRet dalGetStandby(WEB_NTWK_VAR *pWebVar);
#endif


/** Add the L2tpAcIntfConfigObject object
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 *
 * @return CmsRet enum.
 */
CmsRet dalL2tpAc_addL2tpAcInterface(const WEB_NTWK_VAR *webVar);
CmsRet dalL2tpAc_addL2tpAcInterface_igd(const WEB_NTWK_VAR *webVar);
CmsRet dalL2tpAc_addL2tpAcInterface_dev2(const WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalL2tpAc_addL2tpAcInterface(v)     dalL2tpAc_addL2tpAcInterface_igd((v))
#elif defined(SUPPORT_DM_HYBRID)
#define dalL2tpAc_addL2tpAcInterface(v)     dalL2tpAc_addL2tpAcInterface_igd((v))
#elif defined(SUPPORT_DM_PURE181)
#define dalL2tpAc_addL2tpAcInterface(v)     dalL2tpAc_addL2tpAcInterface_dev2((v))
#elif defined(SUPPORT_DM_DETECT)
#define dalL2tpAc_addL2tpAcInterface(v)  (cmsMdm_isDataModelDevice2() ? \
                                     dalL2tpAc_addL2tpAcInterface_dev2((v)) : \
                                     dalL2tpAc_addL2tpAcInterface_igd((v)))
#endif

/** Delete the L2tpAcIntfConfigObject and associated WanPppConnection object
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 *
 * @return CmsRet enum.
 */
CmsRet dalL2tpAc_deleteL2tpAcInterface(const WEB_NTWK_VAR *webVar);
CmsRet dalL2tpAc_deleteL2tpAcInterface_igd(const WEB_NTWK_VAR *webVar);
CmsRet dalL2tpAc_deleteL2tpAcInterface_dev2(const WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalL2tpAc_deleteL2tpAcInterface(v)   dalL2tpAc_deleteL2tpAcInterface_igd((v))
#elif defined(SUPPORT_DM_HYBRID)
#define dalL2tpAc_deleteL2tpAcInterface(v)   dalL2tpAc_deleteL2tpAcInterface_igd((v))
#elif defined(SUPPORT_DM_PURE181)
#define dalL2tpAc_deleteL2tpAcInterface(v)   dalL2tpAc_deleteL2tpAcInterface_dev2((v))
#elif defined(SUPPORT_DM_DETECT)
#define dalL2tpAc_deleteL2tpAcInterface(v)  (cmsMdm_isDataModelDevice2() ? \
                                     dalL2tpAc_deleteL2tpAcInterface_dev2((v)) : \
                                     dalL2tpAc_deleteL2tpAcInterface_igd((v)))
#endif

#ifdef SUPPORT_SAMBA
/**  Adds a user account to etc/passwd and smbpasswd
 *
 * @param webVar (IN) With username,password and homedir pathset for account to be created.
 * @param erroStr (OUT) message with failurei cause   .
 * @return CmsRet
 */
CmsRet dalStorage_addUserAccount(const WEB_NTWK_VAR *webVar, char**errorStr);

/** Deletes a user account from the system
 *
 * @param userName (IN) name with which user account has to be created.
 * @return CmsRet
 */
CmsRet dalStorage_deleteUserAccount(const char *userName);
#endif /* SUPPORT_SAMBA */


#ifdef DMP_X_BROADCOM_COM_AUTODETECTION_1
/** This function will return TRUE if any layer 2 configuration object has LimitedConnected set to TRUE,
 * @return UBOOL8
 */
UBOOL8 dalAutoDetect_isAutoDetectEnabled(void);

/**  Set the LimitedConnections in the layer 2 objects. For now, just EthWAN and MoCAWan
 *
 * @param isEnabled (IN) Boolean if TRUE, auto detect enabled.
 * @return CmsRet
 */
CmsRet dalAutoDetect_setAutoDetectionFlag(UBOOL8 isEnabled);

/**  Enable all WAN connection object when auto detect is enabled.
 *
 * @return CmsRet
 */
CmsRet dalAutoDetect_enableAllWanConn(void);

#endif /* DMP_X_BROADCOM_COM_AUTODETECTION_1 */

#if defined(DMP_X_BROADCOM_COM_MCAST_1)
/** Add a snooping exception to the Multicast config
 *
 * @param char* ip    IP Address of exception
 * @param char* mask  Mask to use when applying except
 *
 * @return CmsRet
 */
CmsRet dalMulticast_AddException(char* ip, char* mask);

/** Set multicast configuration.
 *
 * @param char* rmList space separated string of address/mask (formatted as x.x.x.x/##) to remove
 *
 * @return CmsRet
 */
CmsRet dalMulticast_RemoveException(char* rmList);

/** Add a snooping exception to the Multicast config
 *
 * @param char* ip    IP Address of exception
 * @param char* mask  Mask to use when applying except
 *
 * @return CmsRet
 */
CmsRet dalMulticast_AddExceptionMld(char* ip, char* mask);

/** Set multicast configuration.
 *
 * @param char* rmList space separated string of address/mask (formatted as xxxx:xxxx::xxxx/###) to remove
 *
 * @return CmsRet
 */
CmsRet dalMulticast_RemoveExceptionMld(char* rmList);
#endif

#if defined(DMP_X_BROADCOM_COM_IGMP_1) || defined(DMP_X_BROADCOM_COM_MLD_1) || defined(DMP_X_BROADCOM_COM_MCAST_1)
/** Set multicast configuration.
 *
 * @param pWebVar (IN) multicast config web var.
 *
 * @return CmsRet
 */
CmsRet dalSetMulticastCfg(const PWEB_NTWK_VAR pWebVar);

/** Get multicast configuration.
 *
 * @param pWebVar (IN/OUT) multicast config web var.
 *
 * @return CmsRet
 */
CmsRet dalGetMulticastCfg(WEB_NTWK_VAR *pWebVar);
#endif

#ifdef DMP_X_BROADCOM_COM_GPONWAN_1
/**  Get the Gpon WANDevice iidStack
 *
 * @param gponWanIidStack (IN/OUT) Gpon WANDevice IidStack
 * @return CmsRet
 */
CmsRet dalGpon_getGponWanIidStatck(InstanceIdStack *gponWanIidStack);


/** Config (Add/Delete) the Gpon Interface object by enable/disable the gpon link config object
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 * @param isAdd (IN) Boolean if TRUE, add the gpon wan interface; FALSE, disable gpon interface.
 *
 * @return CmsRet enum.
 */
CmsRet dalGpon_configGponInterface(const WEB_NTWK_VAR *webVar, UBOOL8 isAdd);



/** find the gponLinkCfg object with the given layer 2 ifName. Uses rut function
 *
 * @param ifName (IN) layer 2 ifName of gponLinkCfg to find.
 * @param iidStack (OUT) iidStack of the gponLinkCfg object found.
 * @param dslLinkCfg (OUT) if not null, this will contain a pointer to the found
 *                         gponLinkCfg object.  Caller is responsible for calling
 *                         cmsObj_free() on this object.
 *
 * @return UBOOL8 indicating whether the desired gponLinkCfg object was found.
 */
UBOOL8 dalGpon_getGponLinkByIfName(char *ifName, InstanceIdStack *iidStack, WanGponLinkCfgObject **gponLinkCfg);

/**  Get the WAN service Oid and iidStack
 *
 * @param InstanceIdStack *gponLinkCfgIid (IN) GPON Link Config
 *                        IidStack
 * @param MdmObjectId *oid (OUT)
 * @param InstanceIdStack *iidStack (OUT)
 * @return CmsRet
 */
CmsRet dalGpon_getServiceOidAndIidStack(const InstanceIdStack *gponLinkCfgIid,
                                        MdmObjectId *oid, InstanceIdStack *iidStack);

/**  Get the WAN service Parameters
 *
 * @param GponWanServiceParams *pServiceParams (OUT)
 * @param MdmObjectId *oid (IN)
 * @param InstanceIdStack *iidStack (IN)
 * @return CmsRet
 */
CmsRet dalGpon_getWanServiceParams(const MdmObjectId oid, const InstanceIdStack *iidStack,
                                   GponWanServiceParams *pServiceParams);

/**  Get the WAN service Layer-2 If name
 *
 * @param char *pL2Ifname (OUT)
 * @param MdmObjectId *oid (IN)
 * @param InstanceIdStack *iidStack (IN)
 * @return CmsRet
 */
CmsRet dalGpon_getWanServiceL2IfName(const MdmObjectId oid,
                                     const InstanceIdStack *iidStack,
                                     char *pL2Ifname);
#endif /* DMP_X_BROADCOM_COM_GPONWAN_1 */


#ifdef DMP_X_BROADCOM_COM_EPONWAN_1
/**  Get the Epon WANDevice iidStack
 *
 * @param eponWanIidStack (IN/OUT) Epon WANDevice IidStack
 * @return CmsRet
 */
CmsRet dalEpon_getEponWanIidStatck(InstanceIdStack *eponWanIidStack);


/** Add the Epon Interface object by enable/disable the epon intf config object
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 *
 * @return CmsRet enum.
 */
CmsRet dalEpon_addEponInterface(const WEB_NTWK_VAR *webVar);


/** Delete the Epon Interface object by enable/disable the epon intf config object
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 *
 * @return CmsRet enum.
 */
CmsRet dalEpon_deleteEponInterface(const WEB_NTWK_VAR * webVar);


/** find the eponIntfCfg object with the given layer 2 ifName. Uses rut function
 *
 * @param ifName (IN) layer 2 ifName of eponIntfCfg to find.
 * @param iidStack (OUT) iidStack of the eponLinkCfg object found.
 * @param dslLinkCfg (OUT) if not null, this will contain a pointer to the found
 *                         eponIntfCfg object.  Caller is responsible for calling
 *                         cmsObj_free() on this object.
 *
 * @return UBOOL8 indicating whether the desired eponLinkCfg object was found.
 */
UBOOL8 dalEpon_getEponIntfByIfName(char *ifName, InstanceIdStack *iidStack, WanEponIntfObject **eponIntfCfg);



/** find the eponLinkCfg object with the given layer 2 ifName. Uses rut function
 *
 * @param ifName (IN) layer 2 ifName of eponIntfCfg to find.
 * @param iidStack (OUT) iidStack of the eponLinkCfg object found.
 * @param eponLinkCfg (OUT) if not null, this will contain a pointer to the found
 *                         eponIntfCfg object.  Caller is responsible for calling
 *                         cmsObj_free() on this object.
 *
 * @return UBOOL8 indicating whether the desired eponLinkCfg object was found.
 */
UBOOL8 dalEpon_getEponLinkByIfName(char *ifName, InstanceIdStack *iidStack, WanEponLinkCfgObject **eponLinkCfg);

#endif /* DMP_X_BROADCOM_COM_EPONWAN_1 */

#ifdef DMP_X_BROADCOM_COM_NFC_1
/** configure NFC.
 *
 * @param enable          (IN)
 *
 * @return CmsRet enum.
 */
CmsRet dalNfcCfg(const char *enable);


/** get NFC information.
 *
 * @param info      (IN)
 *
 * @return CmsRet enum.
 */
CmsRet dalGetNfc(char *info);
#endif


#ifdef DMP_DEVICE2_SM_BASELINE_1
CmsRet dalModSw_getAvailableEE(NameList **ifList);
#endif

#ifdef DMP_X_BROADCOM_COM_SNMP_1
CmsRet cmsDal_getCurrentSnmpCfg(WEB_NTWK_VAR *webVar);
CmsRet cmsDal_setSnmpCfg(const WEB_NTWK_VAR *webVar);
#endif /* DMP_X_BROADCOM_COM_SNMP_1 */

#ifdef DMP_X_BROADCOM_COM_DBUSREMOTE_1
void cmsDal_getDbusRemoteCfg(WEB_NTWK_VAR *webVar);
CmsRet cmsDal_setDbusremoteCfg(const WEB_NTWK_VAR *webVar);
#endif




/** Get list of available Wifi interfaces for WAN side
 *
 * @param ifList  (OUT) A NameList list of available interface.  Caller is
 *                      responsible for freeing the list by calling
 *                      cmsDal_freeNameList().
 *
 * @return CmsRet enum.
 */
CmsRet dalWifiWan_getAvailableL2WlIntf(NameList **ifList);
CmsRet dalWifiWan_getAvailableL2WlIntf_igd(NameList **ifList);
CmsRet dalWifiWan_getAvailableL2WlIntf_dev2(NameList **ifList);

#if defined(SUPPORT_DM_LEGACY98)
#define dalWifiWan_getAvailableL2WlIntf(i)  dalWifiWan_getAvailableL2WlIntf_igd((i))
#elif defined(SUPPORT_DM_HYBRID)
#define dalWifiWan_getAvailableL2WlIntf(i)  dalWifiWan_getAvailableL2WlIntf_igd((i))
#elif defined(SUPPORT_DM_PURE181)
#define dalWifiWan_getAvailableL2WlIntf(i)  dalWifiWan_getAvailableL2WlIntf_dev2((i))
#elif defined(SUPPORT_DM_DETECT)
#define dalWifiWan_getAvailableL2WlIntf(i)  (cmsMdm_isDataModelDevice2() ? \
                               dalWifiWan_getAvailableL2WlIntf_dev2((i)) :  \
                               dalWifiWan_getAvailableL2WlIntf_igd((i)))
#endif


/* Get WanWifiIntfObject by Wl wan interface name
 *
 * @param ifname     (IN)  The interface name.
 * @param iidStack   (OUT) iidStack of the wlIntfCfg object found.
 * @param wlIntfCfg  (OUT) if not null, this will contain a pointer to the found
 *                         wlIntfCfg object.  Caller is responsible for calling
 *                         cmsObj_free() on this object.
 *
 * @return UBOOL8 indicating whether the desired wlIntfCfg object was found.
 */
UBOOL8 dalWifiWan_getWlIntfByIfName(char *ifName, InstanceIdStack *iidStack, WanWifiIntfObject **wlIntfCfg);


/** Add the WanWifiIntfObject object
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 *
 * @return CmsRet enum.
 */
CmsRet dalWifiWan_addWlInterface(const WEB_NTWK_VAR *webVar);
CmsRet dalWifiWan_addWlInterface_igd(const WEB_NTWK_VAR *webVar);
CmsRet dalWifiWan_addWlInterface_dev2(const WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalWifiWan_addWlInterface(w)  dalWifiWan_addWlInterface_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define dalWifiWan_addWlInterface(w)  dalWifiWan_addWlInterface_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define dalWifiWan_addWlInterface(w)  dalWifiWan_addWlInterface_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define dalWifiWan_addWlInterface(w)  (cmsMdm_isDataModelDevice2() ? \
                                 dalWifiWan_addWlInterface_dev2((w)) :  \
                                 dalWifiWan_addWlInterface_igd((w)))
#endif


/** Delete the WanWifiIntfObject object
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 *
 * @return CmsRet enum.
 */
CmsRet dalWifiWan_deleteWlInterface(const WEB_NTWK_VAR *webVar);
CmsRet dalWifiWan_deleteWlInterface_igd(const WEB_NTWK_VAR *webVar);
CmsRet dalWifiWan_deleteWlInterface_dev2(const WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalWifiWan_deleteWlInterface(w)  dalWifiWan_deleteWlInterface_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define dalWifiWan_deleteWlInterface(w)  dalWifiWan_deleteWlInterface_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define dalWifiWan_deleteWlInterface(w)  dalWifiWan_deleteWlInterface_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define dalWifiWan_deleteWlInterface(w)  (cmsMdm_isDataModelDevice2() ? \
                                 dalWifiWan_deleteWlInterface_dev2((w)) :  \
                                 dalWifiWan_deleteWlInterface_igd((w)))
#endif


/** Set the Wifi Wan URE mode
 *
 * @param ureMode (IN) 0: disabled; 1: Range Extender; 2: Travel Router
 */
CmsRet dalWifiWan_setUreMode(SINT32 ureMode);




CmsRet dalWan_getAdslFlags_igd(UINT32 *adslFlags);
CmsRet dalWan_getAdslFlags_dev2(UINT32 *adslFlags);
CmsRet dalWan_setAdslFlags_igd(UINT32 adslFlags);
CmsRet dalWan_setAdslFlags_dev2(UINT32 adslFlags);

#if defined(SUPPORT_DM_LEGACY98)
#define dalWan_getAdslFlags(w)  dalWan_getAdslFlags_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define dalWan_getAdslFlags(w)  dalWan_getAdslFlags_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define dalWan_getAdslFlags(w)  dalWan_getAdslFlags_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define dalWan_getAdslFlags(w)  (cmsMdm_isDataModelDevice2() ? \
                                 dalWan_getAdslFlags_dev2((w)) :  \
                                 dalWan_getAdslFlags_igd((w)))
#endif

#if defined(SUPPORT_DM_LEGACY98)
#define dalWan_setAdslFlags(w)  dalWan_setAdslFlags_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define dalWan_setAdslFlags(w)  dalWan_setAdslFlags_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define dalWan_setAdslFlags(w)  dalWan_setAdslFlags_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define dalWan_setAdslFlags(w)  (cmsMdm_isDataModelDevice2() ? \
                                 dalWan_setAdslFlags_dev2((w)) :  \
                                 dalWan_setAdslFlags_igd((w)))
#endif

/** Initiate a speed test
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 *
 * @return CmsRet enum.
 */
CmsRet dalSpdsvc_runSpeedTest(const WEB_NTWK_VAR *webVar);
void dalSpdsvc_getSpeedTestParams(char *varValue);
void dalSpdsvc_getSpeedTestResults(char *varValue);


#ifdef DMP_X_BROADCOM_COM_EPON_1
/** Call appropriate cmsObj_get function to get epon loid info.
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 *
 * @no return.
 */
void cmsDal_getOamLoidPwCfg(WEB_NTWK_VAR *webVar);

/** Call appropriate cmsObj_set function to set epon loid info.
 *
 * @param (IN) pointer to loid to set
 * @param (IN) pointer to password to set
 *
 * @return CmsRet enum.
 */
CmsRet cmsDal_setEponLoidPw(char *eponLoid, char *eponLoidPassword);
#endif

#ifdef DMP_DEVICE2_OPTICAL_1
UBOOL8 dalOptical_getIntfByIfName(const char *ifName, InstanceIdStack *iidStack, OpticalInterfaceObject **optIntfObj);
UBOOL8 dalOptical_getIntfByIfNameEnabled(const char *ifName, InstanceIdStack *iidStack, OpticalInterfaceObject **optIntfObj, UBOOL8 enabled);
CmsRet dalOptical_getInterface(NameList **ifList, const char *ifName, UBOOL8 enable);
#endif /* DMP_DEVICE2_OPTICAL_1 */

#ifdef DMP_X_BROADCOM_COM_OPENVSWITCH_1
CmsRet dalOpenVSCfg(const char *enable, const char *ofControllerAddr,UINT32 ofControllerPort, const char *openVSports );
CmsRet dalGetOpenVS(char *info);
UBOOL8 dalIsOpenVSEnabled();
#endif

/** Get current STUN configuration parameters.
 *
 * @param webData (IN/OUT) pointer to global web data structure
 *
 * @return CmsRet enum.
 */
CmsRet dalStun_getStunCfg(WEB_NTWK_VAR* webVar);

/** Configure STUN.
 *
 * @param webData (IN) pointer to global web data structure
 *
 * @return CmsRet enum.
 */
CmsRet dalStun_StunConfig(const PWEB_NTWK_VAR pWebVar);

#ifdef DMP_DEVICE2_ETHLAG_1
CmsRet dalEthLag_addInterface(const WEB_NTWK_VAR *webVar);
CmsRet dalEthLag_deleteInterface(const WEB_NTWK_VAR *webVar);
CmsRet dalEthLag_getInterface(NameList **ifList);
CmsRet dalEthLag_isEthIntfPartOfEthLag(const char *ethIfName);
#endif /* DMP_DEVICE2_ETHLAG_1 */



#endif /* __CMS_DAL_H__ */
