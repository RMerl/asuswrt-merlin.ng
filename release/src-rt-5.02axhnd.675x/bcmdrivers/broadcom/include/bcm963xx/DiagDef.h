/*
<:copyright-BRCM:2004:DUAL/GPL:standard

   Copyright (c) 2004 Broadcom 
   All Rights Reserved

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

/*******************************************************************
 * DiagDef.h
 * 
 *	Description:
 *		Diag definitions
 *
 * $Revision: 1.35 $
 *
 * $Id: DiagDef.h,v 1.35 2009/01/08 23:54:48 tonytran Exp $
 *
 * $Log: DiagDef.h,v $
 * Revision 1.35  2009/01/08 23:54:48  tonytran
 * Added filters to Status Window and Proxy Client; Added options for Parser to parse a certain statuses; Added an option to download the binary status file from the Proxy Server; Fixed a reconnecting problem b/w the Proxy Client and Server which occur sometimes when the Client got timeout from getting chipId
 *
 * Revision 1.34  2008/05/23 05:31:56  tonytran
 * Added VDSL AFE test configuration support
 * Display Nitro status correctly for VDSL2
 * Display PhyR status per direction in State Window when either US or DS PhyR is enabled and display PhyR status and counters in the Counter Dialog Box
 * Added mibgettofile command
 * Send the test reverb command before file download to prevent the 6368 board crash when file download is initiated during showtime
 *
 * Revision 1.33  2008/03/11 19:11:39  tonytran
 * Added an option to File menu to unlock a connection from another session and start a new session. This changes along with the 20m_rc4 driver(to be released) will eliminate the issue with DslDiags connection being disconnected by another session by accident
 *
 * Revision 1.32  2007/11/14 23:33:55  tonytran
 * Added VDSL2 configuration option:  profiles and  modulation selection and US0 enable/disable
 *
 * Revision 1.31  2007/10/17 07:46:01  tonytran
 * Added Generic Profiling support, DslDiags session lock option, and improved dynamic Cycle Profile graph display
 *
 * Revision 1.30  2007/03/05 18:33:28  tonytran
 * Added DIAG_DEBUG_CMD_SET_L2_TIMEOUT
 *
 * Revision 1.29  2007/02/22 05:00:17  ilyas
 * Added logr - log after reset command to do TEQ logging after PHY reset or download
 *
 * Revision 1.28  2006/08/08 20:04:14  dadityan
 * ^M clean up
 *
 * Revision 1.27  2006/08/08 08:01:44  dadityan
 * Diags Pass OEM Param
 *
 * Revision 1.26  2006/06/12 22:26:11  ilyas
 * Updated version number and made release 21
 *
 * Revision 1.25  2006/03/31 16:32:41  ovandewi
 * changes for PLN and NL from MIB
 *
 * Revision 1.24  2006/03/31 09:08:58  dadityan
 * Added LOG_CMD_CFG_PHY2
 *
 * Revision 1.23  2005/07/14 23:43:20  ilyas
 * Added command to start data logging
 *
 * Revision 1.22  2004/10/16 23:43:19  ilyas
 * Added playback resume command
 *
 * Revision 1.21  2004/10/16 23:24:08  ilyas
 * Improved FileRead command support for LOG file playback (RecordTest on the board)
 *
 * Revision 1.20  2004/04/28 16:52:32  ilyas
 * Added GDB frame processing
 *
 * Revision 1.19  2004/03/10 22:26:53  ilyas
 * Added command-line parameter for IP port number.
 * Added proxy remote termination
 *
 * Revision 1.18  2004/01/24 23:41:37  ilyas
 * Added DIAG_DEBUG_CMD_LOG_SAMPLES debug command
 *
 * Revision 1.17  2003/11/19 02:25:45  ilyas
 * Added definitions for LOG frame retransmission, time, ADSL2 plots
 *
 * Revision 1.16  2003/11/14 18:46:05  ilyas
 * Added G992p3 debug commands
 *
 * Revision 1.15  2003/10/02 19:50:41  ilyas
 * Added support for buffering data for AnnexI and statistical counters
 *
 * Revision 1.14  2003/09/03 19:45:11  ilyas
 * To refuse connection with older protocol versions
 *
 * Revision 1.13  2003/08/30 00:12:39  ilyas
 * Added support for running chip test regressions via DslDiags
 *
 * Revision 1.12  2003/08/12 00:19:28  ilyas
 * Improved image downloading protocol.
 * Added DEBUG command support
 *
 * Revision 1.11  2003/04/11 00:37:24  ilyas
 * Added DiagProtoFrame definition
 *
 * Revision 1.10  2003/03/25 00:10:07  ilyas
 * Added command for "long" BERT test
 *
 * Revision 1.9  2003/01/30 03:29:32  ilyas
 * Added PHY_CFG support and fixed printing showtime counters
 *
 * Revision 1.8  2002/12/16 20:56:38  ilyas
 * Added support for binary statuses
 *
 * Revision 1.7  2002/12/06 20:19:13  ilyas
 * Added support for binary statuses and scrambled status strings
 *
 * Revision 1.6  2002/11/05 00:18:27  ilyas
 * Added configuration dialog box for Eye tone selection.
 * Added Centillium CRC workaround to AnnexC config dialog
 * Bit allocation update on bit swap messages
 *
 * Revision 1.5  2002/07/30 23:23:43  ilyas
 * Implemented DIAG configuration command for AnnexA and AnnexC
 *
 * Revision 1.4  2002/07/30 22:47:15  ilyas
 * Added DIAG command for configuration
 *
 * Revision 1.3  2002/07/15 23:52:51  ilyas
 * iAdded switch RJ11 pair command
 *
 * Revision 1.2  2002/04/25 17:55:51  ilyas
 * Added mibGet command
 *
 * Revision 1.1  2002/04/02 22:56:39  ilyas
 * Support DIAG connection at any time; BERT commands
 *
 *
 ******************************************************************/

#if !defined(_DIAGDEF_H_)
#define _DIAGDEF_H_

#if defined(__cplusplus)
extern "C" {
#endif
#define	LOG_PROTO_ID				"*L"

#define	DIAG_PARTY_ID_MASK			0x01
#define	LOG_PARTY_CLIENT			0x01
#define	LOG_PARTY_SERVER			0x00
#define	DIAG_PARTY_LINEID_MASK		0x06
#define	DIAG_PARTY_LINEID_SHIFT		1

#define DIAG_TYPE_SHIFT 28
#define DIAG_TYPE_MASK (3 << DIAG_TYPE_SHIFT)
#define DIAG_PARTY_TYPE_MASK 0x30
#define DIAG_PARTY_TYPE_SHIFT 4

#define DIAG_PARTY_TYPE_SEND_SHIFT  6
#define DIAG_PARTY_TYPE_SEND_MASK   (3 << DIAG_PARTY_TYPE_SEND_SHIFT)
#define DIAG_TYPE_CMD_SHIFT         30
#define DIAG_TYPE_CMD_MASK          (3 << DIAG_TYPE_CMD_SHIFT)

#define	DIAGS_LINE_SHIFT		    31
#define	DIAGS_LINE_MASK		    (1 << DIAGS_LINE_SHIFT)


#define	DIAG_DSL_CLIENT         0x0
#define	DIAG_WLAN_CLIENT        0x1
#define	DIAG_XTM_CLIENT         0x2

#define	DIAG_DATA_DSL_ID        0x1
#define	DIAG_DATA_XTM_ID		0x2
#define	DIAG_DATA_WLAN_ID		0x4



#define	DIAG_DATA_MASK				0x0E
#define	DIAG_DATA_LOG				0x02
#define	DIAG_DATA_EYE				0x04
#define	DIAG_DATA_LOG_TIME			0x08
#define DIAG_DATA_GUI_ID			0x10
#define DIAG_DATA_GDB_ID			0x20
#define DIAG_DATA_TCP_ID			DIAG_PARTY_ID_MASK // <-- this flag is used when running LOG_CMD_COMMAND. Reusing DIAG_PARTY_ID_MASK bit which is not expected to be in use
#define DIAG_LOCK_SESSION			0x40
#define DIAG_FORCE_DISCONNECT		0x80
#define	DIAG_DATA_EX				0x80
#define	DIAG_PARTY_ID_MASK_EX		(DIAG_DATA_EX | DIAG_PARTY_ID_MASK)
#define	LOG_PARTY_SERVER_EX			(DIAG_DATA_EX | LOG_PARTY_SERVER)

#define	DIAG_ACK_FRAME_ACK_MASK		0x000000FF
#define	DIAG_ACK_FRAME_RCV_SHIFT	8
#define	DIAG_ACK_FRAME_RCV_MASK		0x0000FF00
#define	DIAG_ACK_FRAME_RCV_PRESENT	0x00010000
#define	DIAG_ACK_TIMEOUT			-1
#define	DIAG_ACK_LEN_INDICATION		-1

#define	LOG_CMD_DIAG_CONNECT_INFO	229
#define	LOG_CMD_DISABLE_CLIENT		230
#define	LOG_CMD_ENABLE_CLIENT		231
#define	LOG_CMD_BONDING				232
#define	LOG_CMD_MIB_GET1			233
#define	LOG_CMD_CFG_PHY3			234
#define	LOG_CMD_CFG_PHY2			235
#define	LOG_CMD_GDB					236
#define	LOG_CMD_PROXY				237
#define	LOG_CMD_RETR				238
#define	LOG_CMD_DEBUG				239
#define	LOG_CMD_BERT_EX				240
#define	LOG_CMD_CFG_PHY				241
#define	LOG_CMD_RESET				242
#define	LOG_CMD_SCRAMBLED_STRING	243
#define	LOG_CMD_EYE_CFG				244
#define	LOG_CMD_CONFIG_A			245
#define	LOG_CMD_CONFIG_C			246
#define	LOG_CMD_SWITCH_RJ11_PAIR	247
#define	LOG_CMD_MIB_GET				248
#define	LOG_CMD_LOG_STOP			249
#define	LOG_CMD_PING_REQ			250
#define	LOG_CMD_PING_RSP			251
#define	LOG_CMD_DISCONNECT			252
#define	LOG_CMD_STRING_DATA			253
#define	LOG_CMD_TEST_DATA			254
#define	LOG_CMD_CONNECT				255

typedef struct _LogProtoHeader {
	unsigned char	logProtoId[2];
	unsigned char	logPartyId;
	unsigned char	logCommmand;
} LogProtoHeader;

#define	LOG_FILE_PORT			5100
#define	LOG_FILE_PORT2			5099
#define	GDB_PORT				5098
#define	GDB_PORT_TCP			5097
#define	LOG_MAX_BUF_SIZE		1400
#define	LOG_MAX_DATA_SIZE		(LOG_MAX_BUF_SIZE - sizeof(LogProtoHeader))

typedef struct {
	LogProtoHeader	diagHdr;
	unsigned char	diagData[LOG_MAX_DATA_SIZE];
} DiagProtoFrame;

// These are for SkbDiagFrameData.target field
#define DIAG_SKB_REROUTE_DATA 0
#define GDB_SKB_REROUTE_DATA 1
#define GUI_SKB_REROUTE_DATA 2
typedef struct {
    /* At the moment, we are restricted to have 4 bytes only, */
    /* effectively, we are overwriting part of udpHdr. */
    /* check usage in BcmAdslDiagLinux.c */
    
    /* Order of frameLen and target fields matters */
    /* ie. expecting frameLen to coincide with udpHdr.Lenght */
    /*     and target to coincide with udpHdr.Checksum */
    unsigned short frameLen;
    unsigned short target; // 0 - diag, 1 - gdb, 2 - gui
    DiagProtoFrame diagFrame;
} SkbDiagFrameData;

#define	DIAG_PROXY_TERMINATE				1
#define	DIAG_PROXY_SET_FILTER				2
#define	DIAG_PROXY_DNLOAD_LOGBINFILE		3
#define	DIAG_PROXY_CMD_LINE				4

#define	DIAG_BONDING_LDSTRDB				1

#define	DIAG_DEBUG_CMD_READ_MEM				1
#define	CMDID_READ_MEM_CHIPID				1
#define	DIAG_DEBUG_CMD_SET_MEM				2
#define	DIAG_DEBUG_CMD_RESET_CONNECTION		3
#define	DIAG_DEBUG_CMD_RESET_PHY			4
#define	DIAG_DEBUG_CMD_RESET_CHIP			5
#define	DIAG_DEBUG_CMD_EXEC_FUNC			6
#define	DIAG_DEBUG_CMD_EXEC_ADSL_FUNC		7
#define	DIAG_DEBUG_CMD_WRITE_FILE			8
#define	DIAG_DEBUG_CMD_G992P3_DEBUG			9
#define	DIAG_DEBUG_CMD_G992P3_DIAG_MODE		10
#define	DIAG_DEBUG_CMD_CLEAR_TIME			11
#define	DIAG_DEBUG_CMD_PRINT_TIME			12
#define	DIAG_DEBUG_CMD_LOG_SAMPLES			13

#define	DIAG_DEBUG_CMD_PLAYBACK_STOP		14
#define	DIAG_DEBUG_CMD_PLAYBACK_RESUME		15

#define	DIAG_DEBUG_CMD_LOG_DATA				16
#define	DIAG_DEBUG_CMD_LOG_AFTER_RESET		 1

#define DIAG_DEBUG_CMD_CLEAREOC_LOOPBACK	17
#define	DIAG_DEBUG_CMD_ANNEXM_CFG			18

#define	DIAG_DEBUG_CMD_PRINT_STAT			21
#define	DIAG_DEBUG_CMD_CLEAR_STAT			22
#define DIAG_DEBUG_CMD_SET_OEM				23
#define DIAG_DEBUG_CMD_SET_L2_TIMEOUT			24
#define DIAG_DEBUG_CMD_STAT_SAVE_LOCAL		25
#define DIAG_DEBUG_CMD_IND_READ_6306			26
#define DIAG_DEBUG_CMD_IND_WRITE_6306			27
#define DIAG_DEBUG_CMD_READ_6306			28
#define DIAG_DEBUG_CMD_WRITE_6306			29
#define DIAG_DEBUG_CMD_START_BUF_TEST		30
#define DIAG_DEBUG_CMD_STOP_BUF_TEST			31
#define DIAG_DEBUG_CMD_SET_RAW_DATA_MODE		32
#define DIAG_DEBUG_CMD_SET_REGRESSION_LOCK	33
#define DIAG_DEBUG_CMD_CLR_REGRESSION_LOCK	34
#define DIAG_DEBUG_CMD_SYNC_CPE_TIME		35
#define DIAG_DEBUG_CMD_SWITCH_PHY_IMAGE	36	/* param1 = 0/1 -> bonding/single line image */
#define DIAG_DEBUG_CMD_SAVE_PREFERRED_LINE	37	/* Save DSL preferred line info in flash */
#define DIAG_DEBUG_CMD_MEDIASEARCH_CFG		38
#define DIAG_DEBUG_CMD_SET_EXTBONDINGDBG_PRINT	39
#define DIAG_DEBUG_CMD_DUMPBUF_CFG			40
#define DIAG_DEBUG_CMD_SET_XTM_LINKUP		41	/* param1 - lineId, param2 - tpsTc */
#define DIAG_DEBUG_CMD_SET_XTM_LINKDOWN		42	/* param1 - lineId */
#define DIAG_DEBUG_CMD_READ_AFEPLLMDIV			43	/* For 63138 */
#define DIAG_DEBUG_CMD_WRITE_AFEPLLMDIV			44	/* param1-pllch01_cfg.Bits.mdiv0, param2-pllch45_cfg.Bits.mdiv1 */
#define DIAG_DEBUG_CMD_CONFIG_BKUPIMAGE			45	/* param1-0/1 ==> Disable/Enable */
#define DIAG_DEBUG_CMD_READ_AFEPLLNDIV			46	/* For 63138 */
#define DIAG_DEBUG_CMD_WRITE_AFEPLLNDIV			47	/* param1-pll_ndiv.Bits.ndiv_int */
#define DIAG_DEBUG_CMD_PHY_TYPE_CFG			48
#define DIAG_DEBUG_CMD_MICRO_INTERRUPT			49	/* param1- number of mSecs */
#define DIAG_DEBUG_CMD_GFAST_TESTMODE			50	/* param1: 1-startRtxTestMode, 2-stopRtxTestMode, 3-startTpsTestMode, 4-stopTpsTestMode */
#define DIAG_DEBUG_CMD_SET_AFEPLLHOLDENABLEBITS		51	/* param1 - set/clear pllch45_cfg.Bits.hold_ch1, param2 - set/clear pllch45_cfg.Bits.enableb_ch1 */
#define DIAG_DEBUG_CMD_READ_AFEPLLMDEL			52	/* param1-pllch01_cfg.Bits.mdiv0/mdiv1, param2-pllch45_cfg.Bits.mdiv0/mdiv1 */
#define DIAG_DEBUG_CMD_TOGGLE_AFEPLLMDEL		53	/* param1-pllch01_cfg.Bits.mdiv0/mdiv1, param2-pllch45_cfg.Bits.mdiv0/mdiv1 */
#define DIAG_DEBUG_CMD_SILENT_MODE			54	/* silent mode on/off */
#define DIAG_DEBUG_CMD_OVERRIDE_2ND_AFEIDS		55	/* Override secondary afeIds */
#define DIAG_DEBUG_CMD_PHY_FROM_TMP			56	/* PHY load from /tmp on/off */
#define	DIAG_DEBUG_CMD_DUMP_MEM				57  /* dump specified memory range to a file */
#define	DIAG_DEBUG_CMD_SECT_OFF				58  /* offsets of the variables used in section dump */


typedef struct {
	unsigned short	cmd;
	unsigned short	cmdId;
	unsigned int	param1;
	unsigned int	param2;
	unsigned char	diagData[1];
} DiagDebugData;

#define	DIAG_TEST_CMD_LOAD					101
#define	DIAG_TEST_CMD_READ					102
#define	DIAG_TEST_CMD_WRITE					103
#define	DIAG_TEST_CMD_APPEND				104
#define	DIAG_TEST_CMD_TEST_COMPLETE			105

#define	DIAG_TEST_FILENAME_LEN				64

/* General kDiagGeneralMsgDbgDataPrint flags */
#define kDiagDbgDataSizeMask					0x00030000
#define kDiagDbgDataSize8					0x00000000
#define kDiagDbgDataSize16					0x00010000
#define kDiagDbgDataSize32					0x00020000
#define kDiagDbgDataSize64					0x00030000

#define kDiagDbgDataSignMask					0x00040000
#define kDiagDbgDataSigned					0x00040000
#define kDiagDbgDataUnsigned					0x00000000

#define kDiagDbgDataFormatMask					0x00080000
#define kDiagDbgDataFormatHex					0x00080000
#define kDiagDbgDataFormatDec					0x00000000

#define kDiagDbgDataQxShift					20
#define kDiagDbgDataQxMask					(0xF << kDiagDbgDataQxShift)
#define kDiagDbgDataQ0					0x00000000
#define kDiagDbgDataQ1					(1 << kDiagDbgDataQxShift)
#define kDiagDbgDataQ4					(4 << kDiagDbgDataQxShift)
#define kDiagDbgDataQ8					(8 << kDiagDbgDataQxShift)
#define kDiagDbgDataQ12					(12 << kDiagDbgDataQxShift)
#define kDiagDbgDataQ15					(0xF << kDiagDbgDataQxShift)

typedef	struct _prefixLengthStruct {
	unsigned short len;
	unsigned short lenComplement;
} PrefixLenStruct;

#define	PREFIX_LEN_STRUCT_SIZE					sizeof(PrefixLenStruct)

typedef	struct _cmdBufStruct {
	PrefixLenStruct	prefixLen;
	LogProtoHeader	diagHdr;
	char			cmdData[LOG_MAX_DATA_SIZE];
} CmdBufStruct;

typedef struct {
	unsigned short	cmd;
	unsigned short	cmdId;
	unsigned int	offset;
	unsigned int	len;
	unsigned int	bufPtr;
	char			fileName[DIAG_TEST_FILENAME_LEN];
} DiagTestData;

typedef struct {
	unsigned int	frStart;
	unsigned int	frNum;
} DiagLogRetrData;

typedef struct {
	unsigned char macAddr[6];
	char	devName[32];
} DiagConnectInfo;

/* shared DSL message codes */

#define kDiagReceivedEocCommand  262
#define kDiagStrPrintf			 433

#define kDiagClearEocMsgLengthMask				0x0000FFFF
#define kDiagClearEocMsgNumMask					0x00FF0000
#define kDiagClearEocMsgDataVolatileMask		0x01000000
#define kDiagClearEocMsgDataVolatile			kDiagClearEocMsgDataVolatileMask
#define kDiagClearEocMsgExtraSendComplete		0x02000000
/* Diags functions on CPE */

#ifndef WINNT

#define VA_NARG( ...) VA_NARG_(__VA_ARGS__, 24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0) 
#define VA_NARG_(...) VA_ARG_N(__VA_ARGS__) 
#define VA_ARG_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,N,...)    N 

#ifdef CONFIG_ARM64
#define ARG2(lo,hi)  ((long)(hi) << 32) | (unsigned int)(lo)

#define DiagStrPrintf(lId,clType,fmt,...) do {						                    \
	static const char fmt_str[] = fmt;									                \
	__DiagStrPrintf(lId, clType, fmt_str, sizeof(fmt_str),  (VA_NARG(__VA_ARGS__))+4,   \
	  ARG2(kDiagReceivedEocCommand | (lId << DIAGS_LINE_SHIFT) | (clType << DIAG_TYPE_SHIFT), kDiagStrPrintf), \
	  ARG2(((((VA_NARG(__VA_ARGS__))+1) << 2) + sizeof(fmt_str)) | kDiagClearEocMsgDataVolatileMask, (VA_NARG(__VA_ARGS__))), \
	  ## __VA_ARGS__);	\
} while (0)

#define DiagStrPrintf1(lId,clType,fmt,...) do {						\
	int fmt_len = strlen(fmt) + 1;									\
    __DiagStrPrintf(lId, clType, fmt, fmt_len, (VA_NARG(__VA_ARGS__))+4,	\
	  ARG2(kDiagReceivedEocCommand | (lId << DIAGS_LINE_SHIFT) | (clType << DIAG_TYPE_SHIFT), kDiagStrPrintf), \
	  ARG2(((((VA_NARG(__VA_ARGS__))+1) << 2) + sizeof(fmt_str)) | kDiagClearEocMsgDataVolatileMask, (VA_NARG(__VA_ARGS__))), \
	  ## __VA_ARGS__);	\
} while (0)

#else
#define ARG2(lo,hi)  lo, hi

#define DiagStrPrintf(lId,clType,fmt,...) do {						\
	static const char fmt_str[] = fmt;									\
    __DiagStrPrintf(lId, clType, fmt_str, sizeof(fmt_str), (VA_NARG(__VA_ARGS__))+4,	\
		kDiagReceivedEocCommand | (lId << DIAGS_LINE_SHIFT) | (clType << DIAG_TYPE_SHIFT), \
		kDiagStrPrintf, ((((VA_NARG(__VA_ARGS__))+1) << 2) + sizeof(fmt_str)) | kDiagClearEocMsgDataVolatileMask,  \
		(VA_NARG(__VA_ARGS__)), ## __VA_ARGS__);	\
} while (0)

#define DiagStrPrintf1(lId,clType,fmt,...) do {						\
	int fmt_len = strlen(fmt) + 1;									\
    __DiagStrPrintf(lId, clType, fmt, fmt_len, (VA_NARG(__VA_ARGS__))+4,	\
		kDiagReceivedEocCommand | (lId << DIAGS_LINE_SHIFT) | (clType << DIAG_TYPE_SHIFT), \
		kDiagStrPrintf, ((((VA_NARG(__VA_ARGS__))+1) << 2) + fmt_len) | kDiagClearEocMsgDataVolatileMask,  \
		(VA_NARG(__VA_ARGS__)), ## __VA_ARGS__);	\
} while (0)
#endif

extern void __DiagStrPrintf(unsigned int lineId, unsigned int clientType, const char *fmt, int fmtLen, int argNum, ...);
extern void DiagWriteStatusInfo(unsigned int cmd, char *p, int n, char *p1, int n1);
extern void DiagWriteStatusShort(unsigned int lineId, unsigned int clientType, unsigned int code, unsigned int value);
extern void DiagWriteStatusLong(unsigned int lineId, unsigned int clientType, unsigned int  msgId, void *ptr, unsigned int len, unsigned int  flags);
extern void DiagWriteFile(unsigned int lineId, unsigned int clientType, char *fname, void *ptr, unsigned int len);
extern void DiagOpenFile(unsigned int lineId, unsigned int clientType, char *fname);
extern void DiagDumpData(unsigned int lineId, unsigned int clientType, void *ptr, unsigned int len, unsigned int  flags);
extern void DiagWriteString(unsigned int lineId, unsigned int clientType,  char *fmt, ...);
extern void BcmDiagsMgrInit(void);
extern void DiagWriteStringV(unsigned int lineId, unsigned int clientType, const char *fmt, void *ap);
void BcmDiagsMgrRegisterClient(unsigned int clientType, void *pCallback);
void BcmDiagsMgrDeRegisterClient(unsigned int clientType);
#endif /* WINNT */

#if defined(__cplusplus)
}
#endif

#endif /* _DIAGDEF_H_ */
