/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * Copyright  (C) 2019-2020  MediaTek Inc. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/****************************************************************************

	Abstract:

	All related IEEE802.11f IAPP + IEEE802.11r IAPP extension.

***************************************************************************/

#ifndef __RTMP_IAPP_H__
#define __RTMP_IAPP_H__

#include "iappdefs.h"
#include "debug.h"

#define IAPP_OS_LINUX
//#define IAPP_OS_VXWORKS

/*
	IAPP issues:

	1. Lack of fast roaming support, particularly required for voice;
	2. Only supports use of RADIUS (in a non-standard way);
*/

/*
	Three levels of support for ESS formation are possible with the IAPP
	capabilities described here:

	Level 1: no administrative or security support;
	Level 2: support for dynamic mapping of BSSID to IP addresses; and
	Level 3: support for encryption and authentication of IAPP messages.

	Level 1 support can be achieved by configuring each AP in the ESS with
	the BSSID to IP address mapping for all other APs in the ESS. This may
	be acceptable for a small ESS.

	Many ESS providers will need levels 2 or 3, which require RADIUS support.
*/

/*
	Note with RALINK driver:

	1. OID to RALINK AP driver;
	2. RT_SIGNAL_STRUC
	3. IAPP daemon must be run after br0 & ra0 built up.
*/

/* ---------------------------- User Definition ------------------------ */

#define FT_KDP_FUNC_SOCK_COMM		/* use socket to communicate with driver */
//#define FT_KDP_FUNC_PKT_ENCRYPT		/* do encryption for each IAPP packet */
#define FT_KDP_SELECT_TIMEOUT		/* use timeout to wait for any packet */
#define FT_KDP_KEY_FROM_DAEMON		/* key is set in daemon */

#ifdef RTL865X_SOC
#define FT_KDP_DEFAULT_IF_ETH		"eth1"
#else

#ifdef IAPP_OS_LINUX
#ifdef OPENWRT_SUPPORT
#define FT_KDP_DEFAULT_IF_ETH		"br0"
#else
#define FT_KDP_DEFAULT_IF_ETH		"br0"
#endif /* CONFIG_SUPPORT_OPENWRT */
#endif


#ifdef IAPP_OS_VXWORKS
#define FT_KDP_DEFAULT_IF_ETH		"mirror0"
#endif

#endif

#ifdef OPENWRT_SUPPORT
#define FT_KDP_DEFAULT_IF_WLAN			"br0" /* used to receive command from WLAN */
#else
#define FT_KDP_DEFAULT_IF_WLAN			"br0" /* used to receive command from WLAN */
#endif /* CONFIG_SUPPORT_OPENWRT */

#define FT_KDP_DEFAULT_IF_WLAN_IOCTL	"ra0" /* ioctl command to WLAN */

/*
	Inter-AP communications present opportunities to an attacker.
	The attacker can use IAPP or forged 802.11 MAC management frames as a
	Denial-of-Service (DoS) attack against a STA state in its AP.
	It can capture MOVE packets to gather information on the STA that is
	roaming. It can act as a rogue AP in the ESS.

	If use RADIUS server, the IAPP entity sends the RADIUS Initiate-Request
	and receives the RADIUS Initiate-Accept or Initiate-Reject.
	If the Initiate-Accept is received, then the IAPP entity initializes
	its data structures, functions, and protocols. If an Initiate-Reject
	is received, the IAPP does not start.

	The RADIUS servers provide two functions, mapping the BSSID of an AP to
	its IP address on the DSM and distribution of keys to the APs to allow
	the encryption of the communications between the APs.

	The function of mapping the BSSID of an AP to its IP address on the DSM
	can also be accomplished by local configuration information or the IETF
	Inverse Address Resolution Protocol (RFC 2390:1998).

	We dont support RADIUS server so use a fixed PSK key to encrypt.
*/
#define FT_KDP_DEFAULT_PTK			"1234567812345678"

#ifdef IAPP_OS_LINUX
#define RT_IOCTL_IAPP				(SIOCIWFIRSTPRIV + 0x01)
#endif // IAPP_OS_LINUX //

#ifdef IAPP_OS_VXWORKS
#define PF_PACKET					AF_INET
#define RT_IOCTL_IAPP				VX_RT_PRIV_IOCTL
#define FT_KDP_WLAN_NAME			"ra"
#define FT_KDP_WLAN_UNIT			0
#define FT_KDP_ETH_NAME				"eth" /* must not use mirror */
#define FT_KDP_BR_ETH_IF_NUM		2 /* eth0 and eth1 */
#define FT_KDP_BR_NAME				"mirror"
#define FT_KDP_BR_UNIT				0
#endif // IAPP_OS_VXWORKS //

#define HASH_TABLE_SIZE		128
#define MAX_NUM_OF_CLIENT	HASH_TABLE_SIZE

#define MAC_ADDR_HASH(Addr)            (Addr[0] ^ Addr[1] ^ Addr[2] ^ Addr[3] ^ Addr[4] ^ Addr[5])
#define MAC_ADDR_HASH_INDEX(Addr)      (MAC_ADDR_HASH(Addr) & (HASH_TABLE_SIZE - 1))

/* ---------------------------- MACRO Definition ---------------------------- */
#define HTON_S(x)				htons(x)
#define NTOH_S(x)				ntohs(x)

/* 16 for extra 8B encryption & 8B-align */
#define IAPP_PKT_ALLOCATE(__pPkt, __Len)				\
				IAPP_MEM_ALLOC(__pPkt, __Len+IAPP_SECURITY_EXTEND_LEN)
#define IAPP_PKT_ZERO(__pPkt, __Len)					\
				IAPP_MEM_ZERO(__pPkt, __Len+IAPP_SECURITY_EXTEND_LEN)

#define IAPP_MEM_ALLOC(__pMem, __Size)					\
				{ os_alloc_mem(NULL, (UCHAR **)(&__pMem), __Size);\
				IAPP_MemAllocNum++; }
#define IAPP_MEM_FREE(__Mem)							\
				{ os_free_mem(NULL, __Mem);				\
				IAPP_MemFreeNum++; }

/* calculate the size to the multiple of 8 */
#define IAPP_ENCRYPTED_DATA_SIZE_CAL(__LenOfAction)				\
	if ((__LenOfAction) & 0x07)										\
		(__LenOfAction) += 8 - ((__LenOfAction) & 0x07);			\
	__LenOfAction += 8;


#define IAPP_MEM_ZERO(__Dst, __Len)			NdisZeroMemory(__Dst, __Len)
#define IAPP_MEM_FILL(__Dst, __Val, __Len)	NdisFillMemory(__Dst, __Len, __Val)
#define IAPP_MEM_MOVE(__Dst, __Src, __Len)	NdisMoveMemory(__Dst, __Src, __Len)

#define IAPP_HEX_DUMP(__pPrompt, __pBuf, __Len)							\
	IAPP_HexDump((CHAR *)__pPrompt, (CHAR *)__pBuf, __Len)

#define IAPP_IOCTL_TO_WLAN(__pCtrlBK, __Param, __pData, __pLen, __ApIdx, __Flags) \
{ \
	IAPP_IoctlToWLAN(__pCtrlBK, __Param, (CHAR *)(__pData),	(INT32 *)(__pLen), __ApIdx, __Flags);\
}

/* ---------------------------- Structure Definition ------------------------ */

/*
	IAPP supports three protocol sequences.

	One is initiated by invoking the IAPP-ADD.request after the APME receives
	an MLME-ASSOCIATE.indication.

	The second is initiated by invoking the IAPPMOVE.request after the APME
	receives an MLME-REASSOCIATE.indication.

	The third is initiated by invoking the IAPP-CACHE.request to cache context
	in neighboring APs to facilitate fast roaming.
*/

/* event log */
#define FT_IP_ADDRESS_SIZE			4

/* IAPP header in the frame body, 6B */
typedef struct PACKED _RT_IAPP_HEADER {

	UCHAR	Version;	/* indicates the protocol version of the IAPP */
	UCHAR	Command;	/* ADD-notify, MOVE-notify, etc. */
	UINT16	Identifier;	/* aids in matching requests and responses */
	UINT16	Length;		/* indicates the length of the entire packet */
} RT_IAPP_HEADER;


/* IAPP Action Frame */
/* ADD notify frame */
typedef struct PACKED _RT_IAPP_ADD_NOTIFY {

	RT_IAPP_HEADER	IappHeader;

	/* the number of octets in the MAC Address */
	UCHAR			AddressLen;

	/* should be sent with a value of zero */
#define FT_KDP_ADD_NOTIFY_RSVD_11R_SUPPORT		0x01
	UCHAR			Rsvd;

	/* the MAC address of the STA that has associated */
	UCHAR			MacAddr[ETH_ALEN];

	/* the integer value of the sequence number of the association request frame */
	/*
		The 802.11 sequence number may be an ambiguous indication of the most
		recent association. But, this information may be useful to an algorithm
		making a determination of the location of the most recent association
		of a STA.
	*/
	UINT16			Sequence;

} RT_IAPP_ADD_NOTIFY;

/* MOVE requset frame */
typedef struct PACKED _RT_IAPP_MOVE_NOTIFY {

	RT_IAPP_HEADER	IappHeader;

	/* the number of octets in the MAC Address */
	UCHAR			AddressLen;

	/* should be sent with a value of zero */
	UCHAR			Rsvd;

	/* the MAC address of the STA that has associated */
	UCHAR			MacAddr[ETH_ALEN];

	/* the integer value of the sequence number of the association request frame */
	UINT16			Sequence;

	/* indicates the number of octets in the Context Block field, always 0 */
	UINT16			LenOfContextBlock;

} RT_IAPP_MOVE_NOTIFY;

/* MOVE response frame */
typedef struct PACKED _RT_IAPP_MOVE_RSP {

	RT_IAPP_HEADER	IappHeader;

	/* the number of octets in the MAC Address */
	UCHAR			AddressLen;

#define IAPP_MOVE_RSP_STATUS_SUCCESS		0
#define IAPP_MOVE_RSP_STATUS_DENY			1
#define IAPP_MOVE_RSP_STATUS_STALE			2
	/* 0 Successful, 1 Move denied, 2 Stale move */
	/*
		FAIL indicates that a RADIUS Access-Reject was received in response
		to the RADIUS Access-Request sent to the RADUS server to look up the
		IP address of the Old AP. (not use the IP in the MOVE-notify frame)
	*/
	UCHAR			Status;

	/* the MAC address of the STA that has associated */
	UCHAR			MacAddr[ETH_ALEN];

	/* the integer value of the sequence number of the association request frame */
	UINT16			Sequence;

	/* indicates the number of octets in the Context Block field, always 0 */
	UINT16			LenOfContextBlock;

} RT_IAPP_MOVE_RSP;

/* SEND-SECURITY-BLOCK frame */
typedef struct PACKED _RT_IAPP_SEND_SECURITY_BLOCK {

	RT_IAPP_HEADER	IappHeader;

	/* first 8 bytes of the ACK nonce */
#define IAPP_SB_INIT_VEC_SIZE		8
	UCHAR			InitVec[8];

	/* indicates the number of octets in the Security Block field */
	UINT16			Length;

	/*
		Contains the security information being forwarded from the
		RADIUS Server through the new AP to the old AP.
	*/
	UCHAR			SB[0];

} RT_IAPP_SEND_SECURITY_BLOCK;

/* no use */
typedef struct PACKED _RT_IAPP_ACK_SECURITY_BLOCK {

	RT_IAPP_HEADER	IappHeader;

	/* copied from the Date/Time stamp */
	UCHAR			InitVec[8];

	/*
		Content of the New-AP-ACK-Authenticator information element that
		the old AP received in the Security Block.
	*/
	UCHAR			NewApAckAuth[48];

} RT_IAPP_ACK_SECURITY_BLOCK;

/* private frame */
typedef struct PACKED _RT_IAPP_INFORMATION {

#define IAPP_INFO_TYPE_BC		0
#define IAPP_INFO_TYPE_REQ		1
#define IAPP_INFO_TYPE_RSP		2
#define IAPP_INFO_TYPE_MAX_NUM	3

	RT_IAPP_HEADER	IappHeader;

	/* indicates the number of octets in the Information Block field */
	UINT16			Length;

	/* contains the AP information being forwarded */
	UCHAR			IB[0];

} RT_IAPP_INFORMATION;

typedef struct PACKED _RT_IAPP_SECURITY_MONITOR {

	RT_IAPP_HEADER			IappHeader;
	RT_802_11_EVENT_TABLE	EvtTab;
} RT_IAPP_SECURITY_MONITOR, *PRT_IAPP_SECURITY_MONITOR;


/* Event structure between daemon and driver */
#define FT_KDP_EVT_HEADER_SIZE		sizeof(FT_KDP_EVT_HEADER)

typedef struct _OID_REQ {

	INT32		OID;
	INT32		Len;
	CHAR		Buf[0];
} OID_REQ, *POID_REQ;

typedef struct _MSG_BUF {

	INT32		MsgType;
	INT32		MsgSubType;
	CHAR		Buf[1024];
} MSG_BUF, *PMSG_BUF;


/* Layer 2 Update frame to switch/bridge */
typedef struct PACKED _FT_ETH_HEADER {

	UCHAR	DA[ETH_ALEN];
	UCHAR	SA[ETH_ALEN];
	UINT16	Len;

} FT_ETH_HEADER;

/* For any Layer2 devices, e.g., bridges, switches and other APs, the frame
   can update their forwarding tables with the correct port to reach the new
   location of the STA */
typedef struct PACKED _RT_IAPP_L2_UPDATE_FRAME {

	FT_ETH_HEADER ETH;
	UCHAR	DSAP;			/* must be NULL */
	UCHAR	SSAP;			/* must be NULL */
	UCHAR	Control;		/* reference to IEEE Std 802.2 */
	UCHAR	XIDInfo[3];		/* reference to IEEE Std 802.2 */
} RT_IAPP_L2_UPDATE_FRAME, *PRT_IAPP_L2_UPDATE_FRAME;


/* RRB protocol */
typedef struct PACKED _FT_RRB_FRAME
{
	/* ethernet header */
	FT_ETH_HEADER ETH;

	/* shall be set to 1 */
#define FT_RRB_FRAME_TYPE		1
	UCHAR	RemoteFrameType;

	/* 0 for Remote Request, and to 1 for Remote Response */
	UCHAR	FTPacketType;

	/* the length in octets of the FT Action Frame field */
	UINT16	FTActionLength;

	/* the BSSID of the current AP */
	UCHAR	ApAddress[ETH_ALEN];

	/*
		The contents of the FT Action frame, from the Category field to the
		end of the Action Frame body.
	*/
#define FT_RRB_HEADER_SIZE	(sizeof(FT_RRB_FRAME))
	UCHAR	FTActionFrame[0];
} FT_RRB_FRAME;

typedef struct _FT_CLIENT_INFO {
	struct _FT_CLIENT_INFO *next;
	INT if_idx;
	UCHAR sta_mac[ETH_ALEN];
	UCHAR ap_mac[ETH_ALEN];
	UCHAR used;
	UCHAR hash_idx;
} FT_CLIENT_INFO;

typedef struct _FT_CLIENT_TABLE {
	FT_CLIENT_INFO *hash[HASH_TABLE_SIZE];
	FT_CLIENT_INFO ft_sta_info[MAX_NUM_OF_CLIENT];
	INT32 ft_sta_table_size;
} FT_CLIENT_TABLE;

/* IAPP control block */
#define IAPP_ENCRYPT_KEY_MAX_SIZE		64
#define MAX_WIFI_COUNT 16
typedef struct _RTMP_IAPP {

	CHAR			IfNameEth[IFNAMSIZ]; /* ethernet interface name */
	CHAR			IfNameWlan[IFNAMSIZ]; /* wireless interface name */
	CHAR			IfNameWlanIoctl[MAX_WIFI_COUNT][IFNAMSIZ]; /* wireless interface name */
	UCHAR			IfNameWlanMAC[MAX_WIFI_COUNT][ETH_ALEN]; /* wireless interface name */
	INT32			IfNameWlanCount;

	struct in_addr	AddrOwn; /* IP address of ethernet interface */
	struct in_addr	AddrNetmask; /* netmask of ethernet interface */
	struct in_addr	AddrBroadcast; /* broadcast address of ethernet interface */


	BOOLEAN			FlgIsRcvRunning; /* if rcv handler is running */

	INT32			SocketUdpSend; /* used to send/rcv IAPP multicast frame */
	INT32			SocketTcpRcv; /* used to rcv unicast frame from a peer */
	INT32			SocketRawBr; /* used to send bridge L2 frame */
	INT32			SocketIoctl; /* used to ioctl() to WLAN driver */
	INT32			SocketRawRRB; /* used in RRB RAW protocol */
	INT32			SocketMonitorIP; /* used to monitor local IP address */

	UCHAR			RecvDrvInfMac[ETH_ALEN]; /*used to receiver messae from driver interface mac*/

#ifdef FT_KDP_FUNC_SOCK_COMM
	INT32			SocketRawDrv; /* used to receive message from driver */
#endif // FT_KDP_FUNC_SOCK_COMM //

#ifndef FT_KDP_FUNC_SOCK_COMM
#ifdef IAPP_OS_LINUX
#define FT_KDP_PIPE_ID_READ			0
#define FT_KDP_PIPE_ID_WRITE		1
	INT32			PipeRawDrv[2]; /* used to receive message from driver */
#endif // IAPP_OS_LINUX //
#endif // FT_KDP_FUNC_SOCK_COMM //

#ifdef IAPP_OS_VXWORKS
	VOID			*pBcCookie[FT_KDP_BR_ETH_IF_NUM]; /* for bridge L2 frame */
	VOID			*pDrvCookieTo; /* for ioctl to WLAN driver */
	VOID			*pDrvCookieFrom; /* for ioctl from WLAN driver */
	VOID			*pRrbCookieTo[FT_KDP_BR_ETH_IF_NUM]; /* used in RRB protocol */
	VOID			*pRrbCookieFrom; /* for ioctl from WLAN driver */

#define IAPP_KDP_PIPE_DRV		"/pipe/IAPP_Drv"
#define IAPP_KDP_PIPE_ETH		"/pipe/IAPP_Eth"
#endif // IAPP_OS_VXWORKS //


#ifdef IAPP_EVENT_LOG
	INT32			MsgId;
	BOOLEAN			FlgIsMsgReady;
#endif // IAPP_EVENT_LOG //


	pid_t			PID; /* IAPP task PID */
	UINT16			PacketIdentifier; /* used in IAPP frame identifier */


#if 1//def FT_KDP_FUNC_PKT_ENCRYPT
	/* common key, ASCII, the last byte must be 0x00 */
	CHAR			CommonKey[IAPP_ENCRYPT_KEY_MAX_SIZE+1];
#endif // FT_KDP_FUNC_PKT_ENCRYPT //

	FT_CLIENT_TABLE SelfFtStaTable;
} RTMP_IAPP, *PRTMP_IAPP;

/* key information */
#define FT_IP_ADDRESS_SIZE				4
#define FT_NONCE_SIZE					8

#define FT_KDP_WPA_NAME_MAX_SIZE		16
#define FT_KDP_R0KHID_MAX_SIZE			48
#define FT_KDP_R1KHID_MAX_SIZE			6
#define FT_KDP_S1KHID_MAX_SIZE			6
#define FT_KDP_PMKR1_MAX_SIZE			32 /* 256-bit key */

#define FT_R1KH_ENTRY_TABLE_SIZE		64
#define FT_R1KH_ENTRY_HASH_TABLE_SIZE	FT_R1KH_ENTRY_TABLE_SIZE

typedef struct PACKED _FT_KDP_PMK_KEY_INFO {

	UCHAR	R0KHID[FT_KDP_R0KHID_MAX_SIZE];
	UCHAR	R0KHIDLen;
	UCHAR	PMKR0Name[FT_KDP_WPA_NAME_MAX_SIZE]; /* an ID that names the PMK-R0 */

	UCHAR	R1KHID[FT_KDP_R1KHID_MAX_SIZE];
	UCHAR	S1KHID[FT_KDP_S1KHID_MAX_SIZE];

	/* reserved field */
	UCHAR	RSV[4];

} FT_KDP_PMK_KEY_INFO;

typedef struct PACKED _FT_KDP_EVT_KEY_ELM
{
	/* must be 65535, Proprietary Information */
#define FT_KDP_ELM_ID_PRI			65535
	UINT16	ElmId;

#define FT_KDP_ELM_PRI_LEN			(sizeof(FT_KDP_EVT_KEY_ELM)-4) 
	UINT16	ElmLen;

	/* must be 0x00 0x0E 0x2E, RALINK */
#define FT_KDP_ELM_PRI_OUI_0		0x00
#define FT_KDP_ELM_PRI_OUI_1		0x0E
#define FT_KDP_ELM_PRI_OUI_2		0x2E
#define FT_KDP_ELM_PRI_OUI_SIZE		3
	UCHAR	OUI[3];

	/* station MAC */
	UCHAR	MacAddr[ETH_ALEN];
	UCHAR	RSV[3];

	/* used in request */
	FT_KDP_PMK_KEY_INFO KeyInfo;

	/* used in response */
	UCHAR	PMKR1Name[FT_KDP_WPA_NAME_MAX_SIZE]; /* an ID that names the PMK-R1 */
	UCHAR	PMKR1[FT_KDP_PMKR1_MAX_SIZE]; /* PMK R1 Key */
	UCHAR	R0KH_MAC[ETH_ALEN]; /* MAC of R0KH */

	/*
		During a Fast BSS Transition a non-AP STA shall negotiate the same
		pairwise cipher suite with Target APs as was negotiated in the FT
		Initial Mobility Domain association. The target AP shall verify
		that the same pairwise cipher suite selector is used, using the
		pairwise cipher suite selector value in the PMK-R1 SA received from
		the R0KH.
	*/
	UCHAR	PairwisChipher[4];
	UCHAR	AkmSuite[4];

	UINT32	KeyLifeTime;
	UINT32	ReassocDeadline;
} FT_KDP_EVT_KEY_ELM;

typedef struct PACKED _FT_KDP_EVT_REASSOC {
	UINT16	SeqNum;
	UCHAR	MacAddr[ETH_ALEN];
	UCHAR	OldApMacAddr[ETH_ALEN];
} FT_KDP_EVT_REASSOC;
/* ---------------------------- API Definition ------------------------ */

VOID IAPP_HexDump(
	CHAR	*pPromptStr,
	CHAR	*pSrcBufVA,
	UINT32	SrcBufLen);

BOOLEAN IAPP_IoctlToWLAN(
	RTMP_IAPP		*pCtrlBK,
	INT32			Param,
	CHAR			*pData,
	INT32			*pDataLen,
	UCHAR			ApIdx,
	INT32			Flags);

BOOLEAN mt_iapp_get_wifi_iface_mac(
	RTMP_IAPP		*pCtrlBK);

INT32 mt_iapp_find_ifidx_by_mac(
	RTMP_IAPP		*pCtrlBK,
	UCHAR			*WifiMAC);

VOID mt_iapp_set_daemon_information(
	RTMP_IAPP		*pCtrlBK,
	pid_t 			*pPidAuth);

VOID mt_iapp_ft_client_table_init(
	RTMP_IAPP		*pCtrlBK);

FT_CLIENT_INFO *mt_iapp_ft_client_look_up(
	FT_CLIENT_TABLE		*pFtTable,
	UCHAR *pAddr);

FT_CLIENT_INFO *mt_iapp_ft_client_insert(
	FT_CLIENT_TABLE		*pFtTable,
	UCHAR 			*pStaAddr,
	UCHAR 			*pApAddr,
	INT32			ApIfIdx);

VOID mt_iapp_ft_client_delete(
	FT_CLIENT_TABLE		*pFtTable,
	UCHAR 			*pStaAddr);

INT32 mt_iapp_find_ifidx_by_sta_mac(
	FT_CLIENT_TABLE		*pFtTable,
	UCHAR			*pStaMAC);

/* Local Variable */
#define IAPP_SHOW_IP_HTONL(__IP)	\
	(htonl(__IP) >> 24) & 0xFF,		\
	(htonl(__IP) >> 16) & 0xFF,		\
	(htonl(__IP) >> 8) & 0xFF,		\
	(htonl(__IP) & 0xFF)

#define IAPP_SHOW_IP(__IP)			\
	(__IP) & 0xFF,					\
	((__IP) >> 8) & 0xFF,			\
	((__IP) >> 16) & 0xFF,			\
	((__IP) >> 24) & 0xFF

#define IAPP_IDENTIFIER_GET(__CB)	(__CB)->PacketIdentifier

#define IAPP_CMD_BUF_ALLOCATE(__pCmdBuf, __pBufMsg, __BufLen)	\
	{															\
		if ((__BufLen) > IAPP_MAX_RCV_PKT_SIZE)					\
		{														\
			printf("iapp> Command Length is too large!\n");		\
			__pBufMsg = NULL;									\
		}														\
		else													\
			__pBufMsg = __pCmdBuf;								\
	}

#define IAPP_RSP_BUF_ALLOCATE(__pRspBuf, __pBufMsg, __BufLen)	\
	{															\
		if ((__BufLen) > IAPP_MAX_RCV_PKT_SIZE)					\
		{														\
			printf("iapp> Response Length is too large!\n");	\
			__pBufMsg = NULL;									\
		}														\
		else													\
			__pBufMsg = __pRspBuf;								\
	}

static __inline__ __uint32_t inet_make_mask(int logmask)
{
	if (logmask)
		return htonl(~((1<<(32-logmask))-1));
	return 0;
}

VOID IAPP_PID_Kill(
	VOID);

VOID IAPP_Task(
	IAPP_IN		VOID		*pContext);

#endif /* __RTMP_IAPP_H__ */

/* End of rtmp_iapp.h */
