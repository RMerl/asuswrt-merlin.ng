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

#if !defined(_DIAGDEF_H_)
#define _DIAGDEF_H_

#if defined(__cplusplus)
extern "C" {
#endif

#define DIAGDEF_H_API_VER   0


#define	LOG_PROTO_ID				"*L"
/* For logPartyId */
#define DIAG_PARTY_ID_MASK          0x01
#define LOG_PARTY_CLIENT            0x01
#define LOG_PARTY_SERVER            0x00
#define DIAG_PARTY_LINEID_SHIFT     1
#define DIAG_PARTY_LINEID_MASK      (3 << DIAG_PARTY_LINEID_SHIFT)
#define DIAG_PARTY_TYPE_SHIFT       4
#define DIAG_PARTY_TYPE_MASK        (3 << DIAG_PARTY_TYPE_SHIFT)    // Support up to 4 clients
#define DIAG_PARTY_TYPE_SEND_SHIFT  DIAG_PARTY_TYPE_SHIFT
#define DIAG_PARTY_TYPE_SEND_MASK   DIAG_PARTY_TYPE_MASK
/* For Status code */
#define DIAG_TYPE_SHIFT             28                              // bit 30, for MIPS core id, bits 29,28 for client Type
#define DIAG_TYPE_MASK              (3 << DIAG_TYPE_SHIFT)          // Support up to 4 clients
#define DIAG_TYPE_CMD_SHIFT         DIAG_TYPE_SHIFT
#define DIAG_TYPE_CMD_MASK          DIAG_TYPE_MASK
#define DIAGS_LINE_SHIFT            31
#define DIAGS_LINE_MASK             (1 << DIAGS_LINE_SHIFT)

#define	DIAG_DSL_CLIENT         0
#define	DIAG_SRDS_ETH_CLIENT    1
#define	DIAG_XTM_CLIENT         2
#define	DIAG_SRDS_PCIE_CLIENT   3
#define	DIAG_MAX_CLIENT         4

#define	DIAG_DATA_DSL_ID        0x1
#define	DIAG_DATA_XTM_ID        0x2
#define	DIAG_DATA_SRDS_ETH_ID   0x4
#define	DIAG_DATA_SRDS_PCIE_ID  0x8


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

#define	LOG_CMD_GET_SERDES_EYEDATA	227
#define	LOG_CMD_SERDES_EYEDATA		228
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
#define	DIAG_PORT_TCP			LOG_FILE_PORT	// Use the same Diag UDP port
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
 #define	CMDID_WREAD_SIZE_MASK       0xF
 #define	CMDID_WREAD_REGION_LSB      4
 #define	CMDID_WREAD_REGION_MASK     (0xF  << CMDID_WREAD_REGION_LSB)
 #define	CMDID_WREAD_REGION_PMD      (0  << CMDID_WREAD_REGION_LSB)
 #define	CMDID_WREAD_REGION_MAC      (1  << CMDID_WREAD_REGION_LSB)
 #define	CMDID_WREAD_REGION_LP_MAC   (2  << CMDID_WREAD_REGION_LSB)
 #define	CMDID_WREAD_REGION_PRAM     (3  << CMDID_WREAD_REGION_LSB)
 #define	CMDID_WREAD_REGION_UBUS     (4  << CMDID_WREAD_REGION_LSB)
 #define	CMDID_WREAD_BCAST_LSB       8
 #define	CMDID_WREAD_BCAST_MASK      (1  << CMDID_WREAD_BCAST_LSB)
 #define	CMDID_WREAD_BCAST           (1  << CMDID_WREAD_BCAST_LSB)
 #define	CMDID_READ_MEM_8BITS        0
 #define	CMDID_READ_MEM_CHIPID       1
 #define	CMDID_READ_MEM_16BITS       2
 #define	CMDID_READ_MEM_32BITS       3

 #define	CMDADDR_REG_LSB             28
 #define	CMDADDR_REG_MASK            (7  << CMDADDR_REG_LSB)
 #define	CMDADDR_REG                 (5  << CMDADDR_REG_LSB)
 #define	CMDADDR_DEVID_LSB           24
 #define	CMDADDR_DEVID_MASK          (0xF  << CMDADDR_DEVID_LSB)
 #define	CMDADDR_LANE_LSB            16
 #define	CMDADDR_LANE_MASK           (0xFF  << CMDADDR_LANE_LSB)
 #define	CMDADDR_LANE_BCAST          (0xFF  << CMDADDR_LANE_LSB)
 #define	CMDADDR_LANE_BCAST1         (0x1F  << CMDADDR_LANE_LSB)
 #define	CMDADDR_LANE_MCAST0         (0xFE  << CMDADDR_LANE_LSB)
 #define	CMDADDR_LANE_MCAST1         (0xFD  << CMDADDR_LANE_LSB)
#define	DIAG_DEBUG_CMD_SET_MEM				2
 #define	CMDID_SET_MEM_32BITS        0
 #define	CMDID_SET_MEM_8BITS         1
 #define	CMDID_SET_MEM_16BITS        2
 #define	CMDID_SET_MEM_32BITS_1      3
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
 #define STAT_SAVE_LOCAL_INIT               1
 #define STAT_SAVE_LOCAL_ENABLE             2
 #define STAT_SAVE_LOCAL_DISABLE            3
 #define STAT_SAVE_LOCAL_WR2DIAG            4   /* Write saved statuses to Diags */
 #define STAT_SAVE_LOCAL_ENABLECONTINOUS    5   /* Enable logging statues continously */
 #define STAT_SAVE_LOCAL_STARTCONTINOUS     6   /* Start logging statues continously */
 #define STAT_SAVE_LOCAL_STOPLOGGING        7
 #define STAT_SAVE_LOCAL_STARTUNTILFULL     8   /* Start logging statues until buffer is full */
 #define STAT_SAVE_LOCAL_STARTUNTILRETRAIN  9   /* Start logging statues until retrain */
 #define STAT_SAVE_LOCAL_PAUSELOGGING       10
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
#define DIAG_DEBUG_CMD_READ_AFEPLL_ChCfg	59	/* param1-chanId(0 - ch01, 1 - ch23, 2 - ch45) to read */
#define DIAG_DEBUG_CMD_WRITE_AFEPLL_ChCfg	60	/* param1-chanId(0 - ch01, 1 - ch23, 2 - ch45) to write; param2-value to be written */
#define DIAG_DEBUG_CMD_TDD_GPIO_CTRL		61	/* param1-0/1 ==> de-activate/activate */
#define DIAG_DEBUG_CMD_PWRBOOST_GPIO_CTRL	62	/* param1-0/1 ==> de-activate/activate */
#define DIAG_DEBUG_CMD_RELAY_GPIO_CTRL		63	/* param1-0/1 ==> de-activate/activate */
#define DIAG_DEBUG_CMD_TIMER_EXPIRE		64	/* param1-0 ==> timer interval */
 #define TIMER_EXPIRE_10MS          1
 #define TIMER_EXPIRE_20MS          2
 #define TIMER_EXPIRE_50MS          5
 #define TIMER_EXPIRE_100MS         10
 #define TIMER_EXPIRE_1000MS        100
#define DIAG_DEBUG_CMD_SERDES_DIAG		65	/* param0-testBitmap(8-eyedata display), param1-laneBitmap */
 #define CMDID_DISPLAY_ON_SERIALCONSOLE 0   /* Display output to the serial console */
 #define CMDID_DISPLAY_IN_DIAG_RAW_DATA 1   /* Display output to Diag in raw data format */
 #define CMDID_DISPLAY_IN_DIAG          2   /* Display output to Diag in console format */
#define DIAG_DEBUG_CMD_SERDES_CMD		66	/* serdes_pmd_uc_cmd: cmdId - cmd, param1 - supp_info, param2 - DSC_SCRATCH */
#define DIAG_DEBUG_CMD_SERDES_CTRL		67	/* Serdes driver operation control */
 #define SRDS_CTRL_EVENT_MONITORING    0x1  /* param1 bit to enable event monitoring */
#define DIAG_DEBUG_CMD_READ_EVENTS		68	/* Read event buffer and report to DslDiags */
#define DIAG_DEBUG_CMD_RESET_UCODE_INFO	69	/* Re-init ucode info (after eval tool ucode download) */
#define DIAG_DEBUG_CMD_EVAL_TEST		70	/* Eval tools started/finished */
#define DIAG_DEBUG_CMD_PAUSE_CON_MONITOR	71	/* Pause Diags Connection monitoring: param1 0/xyz - disable/pause for xyz secsonds */
#define DIAG_DEBUG_CMD_SET_TXEQ_PRESETS 72	/* Set TX equalizer presets (PCIe only?) */
#define DIAG_DEBUG_CMD_FLOW_CONTROL     73	/* flow control on(1)/off(0) for gdb with eval */
#define DIAG_DEBUG_CMD_RESET_GET_CAPS   74	/* system reset and get remote system capabilities */
 #define	SYSCAP_RESET_DONE   0x1
 #define	SYSCAP_PRAM         0x2
 #define	SYSCAP_UCLOAD       0x4
 #define	SYSCAP_PROXY_MASK   0x38
 #define	SYSCAP_PROXY_NONE   0x00
 #define	SYSCAP_PROXY_MDIO   0x08
 #define	SYSCAP_PROXY_FIO    0x18
 #define	SYSCAP_PROXY_SOCKET 0x38
 #define	SYSCAP_MASKED_WRITE 0x40

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

/* Re-use in serdes event data */
#define kDiagDbgSerdesEventBufOverflow			kDiagDbgDataSigned

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

#define kDiagImageTestLoadImage        0     // kDslAfeTestLoadImage
#define kDiagImageTestLoadImageOnly    2     // kDslAfeTestLoadImageOnly

#define diagStatusInfoData             48    // statusInfoData

#define kFirstDiagCommandCode          256
#define kDiagLoadPhyTestCmd            (kFirstDiagCommandCode + 31)

/* common status codes */
#define kDiagDataAvailStatus           279         // kDslDataAvailStatus
#define kDiagReceivedEocCommand        262
#define kDiagGeneralMsgDbgDataPrint    300         // kDslGeneralMsgDbgDataPrint
#define kDiagGeneralMsgDbgFileName     305         // kDslGeneralMsgDbgFileName
#define kDiagGeneralMsgDbgWriteFile    306         // kDslGeneralMsgDbgWriteFile
#define kDiagStrPrintf                 433

/* serdes specific status codes, can't be the same as common ones */
#define kDiagSerdesEventData           350

#define kDiagDbgFileNameDelete            0x00010000  // kDslDbgFileNameDelete

#define kDiagClearEocMsgLengthMask        0x0000FFFF
#define kDiagClearEocMsgNumMask           0x00FF0000
#define kDiagClearEocMsgDataVolatileMask  0x01000000
#define kDiagClearEocMsgDataVolatile      kDiagClearEocMsgDataVolatileMask
#define kDiagClearEocMsgExtraSendComplete 0x02000000
/* Diags functions on CPE */

#ifndef WINNT

#define  DIAG_DRV_SPLIT_MSG      0x80000000
#define  DIAG_DRV_MSG_NUM        0x40000000
#define  DIAG_DRV_LINE_MASK      0x0C000000  // bit 27, 26
#define  DIAG_DRV_LINE_SHIFT     26
#define  DIAG_DRV_TYPE_MASK      0x03000000  // bit 25, 24
#define  DIAG_DRV_TYPE_SHIFT     24

#define  DIAG_SKB_USERS          0x3FFFFFFF
#define  DIAG_DATA_ALIGN(x, y)   ((unsigned char *)(((unsigned long)(x)+(unsigned long)(y)) & ~(y)))
#define  DiagPacketStart(skb)    ((diagSockFrame *) (skb)->head)

#if !defined(CONFIG_BCM_PHY_DIAG) && !defined(CONFIG_BCM_PHY_DIAG_MODULE)

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

void __DiagStrPrintf(unsigned int lineId, unsigned int clientType, const char *fmt, int fmtLen, int argNum, ...);
void DiagWriteStatusInfo(unsigned int cmd, char *p, int n, char *p1, int n1);
void DiagWriteFile(unsigned int lineId, unsigned int clientType, char *fname, void *ptr, unsigned int len);
void DiagOpenFile(unsigned int lineId, unsigned int clientType, char *fname);
void DiagDumpData(unsigned int lineId, unsigned int clientType, void *ptr, unsigned int len, unsigned int  flags);
void DiagWriteString(unsigned int lineId, unsigned int clientType,  char *fmt, ...);

#else	// defined(CONFIG_BCM_PHY_DIAG || defined(CONFIG_BCM_PHY_DIAG_MODULE)

#define CONFIG_BCM_PHY_DIAG_DRV

#define DiagWriteStatusInfo   BcmDiagWriteStatusInfo
#define DiagWriteString       BcmDiagWriteString
#define DiagStrPrintf         BcmDiagWriteString
#define DiagWriteData         BcmDiagWriteData
#define GdbWriteData          BcmDiagGdbWriteData
#define DiagOpenFile          BcmDiagOpenFile
#define DiagWriteFile         BcmDiagWriteFile
#define DiagDumpData          BcmDiagDumpData
#define BcmXdslDiagGetSrvIpAddr  BcmDiagGetSrvIpAddr

#define BcmAdslCoreDiagWriteFile(lineId, fname, ptr, len)	BcmDiagWriteFile(lineId,DIAG_DSL_CLIENT, fname, ptr, len)
#define BcmAdslCoreDiagOpenFile(lineId, fname)			BcmDiagOpenFile(lineId,DIAG_DSL_CLIENT, fname)

void BcmDiagWriteString(unsigned int lineId, unsigned int clientType,  char *fmt, ...);
int BcmDiagWriteData(unsigned int  cmd, char *buf0, int len0, char *buf1, int len1);
void BcmDiagWriteStatusInfo(unsigned int cmd, char *p, int n, char *p1, int n1);
#if defined(LINUX_FW_EXTRAVERSION)
// For DSL driver
#if (LINUX_FW_EXTRAVERSION < 60101)
#define DIAG_WLAN_CLIENT      DIAG_SRDS_ETH_CLIENT
#define DIAG_SERDES_CLIENT    DIAG_SRDS_PCIE_CLIENT
#define DIAG_DATA_WLAN_ID     DIAG_DATA_SRDS_ETH_ID
#define DIAG_DATA_SERDES_ID   DIAG_DATA_SRDS_PCIE_ID
void BcmDiagRegisterClient(unsigned int clientType, void *pCallback);
#else
void BcmDiagRegisterClient(unsigned int clientType, void *pCallback, unsigned int timerInterval);
#endif
#else	// !defined(LINUX_FW_EXTRAVERSION)
void BcmDiagRegisterClient(unsigned int clientType, void *pCallback, unsigned int timerInterval);
#endif
void BcmDiagDeRegisterClient(unsigned int clientType);
int BcmDiagGdbWriteData(char *buf0, int len0);
void BcmDiagReportEocData(unsigned int lineId, unsigned int clientType, int eocId, void *ptr, unsigned int len, unsigned int flags);
void BcmDiagOpenFile(unsigned int lineId, unsigned int clientType, char *fname);
void BcmDiagWriteFile(unsigned int lineId, unsigned int clientType, char *fname, void *ptr, unsigned int len);
void BcmDiagDumpData(unsigned int lineId, unsigned int clientType, void *ptr, unsigned int len, unsigned int  flags);
void BcmDiagEventData(unsigned int lineId, unsigned int clientType, void *ptr, unsigned int len, unsigned int flags);
unsigned int BcmDiagGetSrvIpAddr(void);

#endif	// !defined(CONFIG_BCM_PHY_DIAG) && !defined(CONFIG_BCM_PHY_DIAG_MODULE)

#endif /* WINNT */

#if defined(__cplusplus)
}
#endif

#endif /* _DIAGDEF_H_ */
