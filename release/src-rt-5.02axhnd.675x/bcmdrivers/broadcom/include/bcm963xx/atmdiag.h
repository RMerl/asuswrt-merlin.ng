/*
<:copyright-broadcom 
 
 Copyright (c) 2002 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          16215 Alton Parkway 
          Irvine, California 92619 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
*/

#ifndef __ATMDIAG_H__
#define __ATMDIAG_H__

#if defined(__cplusplus)
extern "C" {
#endif

#define ATM_DIAG_FAIL                       -1
#define ATM_DIAG_PASS                       0
#define ATM_REGADDR			0xFFFE4000
#define ATM_REGSIZE                         0x800
#define ATM_TX_VPI_VCI_CAM_OFFSET           0x500
#define ATM_RX_VPI_VCI_CAM_OFFSET           0x600
#define ATM_TRAFFIC_SHAPER_OFFSET           0x700
#define ATM_TX_STATUS_OFFSET                0x40c
#define ATM_RX_STATUS_OFFSET                0x41c
#define ATM_RX_AAL_STATUS_OFFSET            0x428
#define ATM_MIP_COUNTERS_OFFSET             0x440
#define ATM_UTOPIA_SETTING_OFFSET           0x42c
#define ATM_ADSL_PHY_PORT_SETTING           0x15c
#define UT_MAX_TDT_ENTRIES                  2
#define UT_MAX_MGMT_ENTRIES                 4
#define UT_LINE_RATE                (146200000)  /* 344811 cells/sec, CIT = 29ns */
#define UT_CELL_RATE                (53 * 8)
#define UT_MIN_PCR_SCR              310          /* ~128Kbps */
#define UT_MAX_PCR_SCR              344811       /* ~146.2Kpbs */
#define UT_MIN_MBS                  2
#define UT_MAX_MBS                  200000
#define UT_MAX_VCCS                         16
#define UT_MAX_PHY_PORTS                    2
#define UT_BASE_PORT_NUMBER                 1
#define UT_MIN_QUEUE                        1
#define UT_MAX_QUEUE                        UT_MAX_VCCS
#define UT_MULTI_QUEUE                      1
#define UT_SINGLE_QUEUE                     0
#define UT_MIN_PRIORITY                     1
#define UT_MAX_PRIORITY                     4    /* priority ranging from 1-4 */
#define UT_BUFFER_SIZE                      20
#define UT_MGMT_IDX                         6
#define UT_ENABLED                          1
#define UT_DISABLED                         0
#define UT_MAX_TD_INDEX                     UT_MAX_VCCS
#define UT_SSTED_TRAILER_SIZE               8
#define UT_DIALED_DIGITS                    2
#define UT_FREE_CELL_Q_SIZE                 800
#define UT_FREE_PKT_Q_SIZE                  800
#define UT_FREE_PKT_Q_BUF_SIZE              1600
#define UT_RX_PKT_Q_SIZE                    800
#define UT_RX_CELL_Q_SIZE                   800
#define UT_AAL5_MAX_SDU_LENGTH              65535
#define UT_TX_FIFO_PRIORITY                 4
#define UT_MIN_DATA_LEN                     48
#define UT_MAX_DATA_LEN                     1500
#define UT_BASE_VPI_NUMBER                  0
#define UT_MAX_VPI_NUMBER                   256
#define UT_BASE_VCI_NUMBER                  32
#define UT_MAX_VCI_NUMBER                   65536
#define UT_UTOPIA_MODE                      1
#define UT_ADSL_MODE                        0
#define UT_UTOPIA_ADSL_MODE                 0x11   /* utopia port 0, adsl port 1 */
#define UT_TOGGLE_DISPLAY_MODE              1
#define UT_TOGGLE_CAPTURE_MODE              0
#define UT_TOGGLE_VERIFICATION_MODE         2
#define UT_TOGGLE_MODE_ON                   1
#define UT_TOGGLE_MODE_OFF                  0
#define UT_DUMP_TX_VPI_VCI_TABLE            1
#define UT_DUMP_RX_VPI_VCI_TABLE            2
#define UT_DISPLAY_STATS                    1
#define UT_CLEAR_STATS                      2
#define UT_TRAFFIC_DESCRIPTOR_DISPLAY       1
#define UT_TRAFFIC_DESCRIPTOR_MODIFY        2
#define UT_PORT_UTOPIA_SETTING              1
#define UT_GLOBAL_UTOPIA_SETTING            2
#define UT_DISPLAY_CAPTURED                 0
#define UT_ERASE_CAPTURED                   1
#define UT_CAPTURED_ERROR_STATS             2
#define UT_PATTERN_INCREMENT                1
#define UT_PATTERN_FIX                      0
#define UT_MODIFY_OPERATION                 1
#define UT_DISPLAY_OPERATION                0
#define DIAG_ATM_MODULE       "bcmatmtest"
#define DIAG_ATM_PROC         "/proc/atmtest"

/* command is made up of 2_bytes_command|2_bytes_index */
/* index is ranging from 0-7 for 8 VCs */
#define UT_PROC_CMD_ADD_VC                  1
#define UT_PROC_CMD_DELETE_VC               2
#define UT_PROC_CMD_START_SEND_VC           3
#define UT_PROC_CMD_SEND_MULTI_VC           4
#define UT_PROC_CMD_STOP_SEND_VC            5
#define UT_PROC_CMD_CAPTURE                 6
#define UT_PROC_CMD_TOGGLE                  7
#define UT_PROC_CMD_GET_STATS               8
#define UT_PROC_CMD_CLEAR_STATS             9
#define UT_PROC_CMD_SEND_MULTI_PRIORITY     10
#define UT_PROC_CMD_MODIFY_TRAFFIC_SHAPER   11
#define UT_PROC_CMD_START_SEND_ALL_VC       12
#define UT_PROC_CMD_ADSL_LOOPBACK           13
#define UT_PROC_CMD_SEND_MANAGEMENT         14
#define UT_PROC_CMD_ADD_MPVC                15
#define UT_PROC_CMD_DELETE_MPVC             16
#define UT_PROC_CMD_START_SEND_MPVC         17
#define UT_PROC_CMD_UTOPIA_SET              18

#define UT_OAM_LB_END_TO_END                10  /* was 1 */
#define UT_OAM_LB_SEGMENT                   11  /* was 2 */
#define UT_OAM_RDI_END_TO_END               3
#define UT_OAM_RDI_SEGMENT                  4
#define UT_VPC_RM_TYPE                      5
#define UT_VCC_RM_TYPE                      6
#define UT_OAM_CRC10_SOFTWARE               0
#define UT_OAM_CRC10_HARDWARE               1
#define UT_TOGGLE_DISPLAY                   0
#define UT_TOGGLE_CAPTURE                   1
#define UT_TOGGLE_VERIFY                    2

#define AP_REG_OFFSET_END                   0x7ff
#define AP_INDIRECT_RAM_ADDRESS_REG      ATM_PROCESSOR_BASE + 0x7c0
#define AP_INDIRECT_RAM_REG              ATM_PROCESSOR_BASE + 0x7c4
#define AP_IRQ_MASK                      AP_INTR_REGS_BASE+0x4
#define AP_IRQ_STATUS                    AP_INTR_REGS_BASE
#define AP_ROUTE_OAM_TO_RCQ              0
#define AP_ROUTE_OAM_TO_MCF              1
#define AP_IR_ASSERT                     1
#define AP_IR_DEASSERT                   0
#define AP_RX_STATUS_ERR_MASK            0x32ecc /* mask out idleCell, vc & unused */
#define AP_RX_AAL_STATUS_ERR_MASK        0x3008  /* only look at rx router stats, discard */

typedef struct utVccCfg {
  UINT8 ulAalType;
  UINT8 ulAtmVccCpcsAcceptCorruptedPdus;
}UT_VCC_CFG, *pUT_VCC_CFG;

typedef struct utTrafficDescrParmEntry {
  UINT32 ulTrafficDescrIndex;
  UINT32 ulTrafficDescrType;
  UINT32 ulTrafficDescrParm1;
  UINT32 ulTrafficDescrParm2;
  UINT32 ulTrafficDescrParm3;
  UINT32 ulTrafficDescrParm4;
  UINT32 ulTrafficDescrParm5;
  UINT32 ulTrafficDescrRowStatus;
  UINT32 ulServiceCategory;
}UT_TRAFFIC_DESCR_PARM_ENTRY,*pUT_TRAFFIC_DESCR_PARM_ENTRY;

typedef struct utMultiSendInfo {
  UINT32 len;
  UINT8  pattern;
  UINT8  dataByte;
  UINT32 numSent;
  UINT32 rate;
  UINT8  circuitType;
  UINT32 cellsPerPdu; 
  UINT32 delay;
  UINT32 txCount;
}UT_MULTISEND_INFO, *pUT_MULTISEND_INFO;

typedef struct utUserSendInfo {
  UINT32 len;
  UINT8  incremental;
  UINT8  dataByte;
  UINT32 rate;
  UINT32 aalType;
  UINT32 delay;
  UINT32 txCount;   /* number of cells/pkt user want to send */
  UINT8  multiQPriority;
  UINT8  basePriority;
  UINT8  numOfQueues;
}UT_USER_SEND_INFO, *pUT_USER_SEND_INFO;

typedef struct utVccAddrInfo {
  ATM_VCC_ADDR vccAddr;
  UINT8 priority;  /* priority of the queue of this VCC */
  UINT8 numOfQueues;
}UT_VCC_ADDR_INFO, *pUT_VCC_ADDR_INFO;

typedef struct utVccListInfo {
  UINT32 handle;
  UINT32 managementHandle;
}UT_VCC_LIST_INFO, *pUT_VCC_LIST_INFO;

typedef struct atmCaptureHdr {
  UINT8 valid;
  UINT8 vpi;
  UINT16 vci;
  UINT8 circuitType;
  UINT8 cid;
  UINT8 uuData8;
  UINT8 uuData5;
  UINT8 ucFlags;
  UINT32 dataLen;
  UINT8 *dataPtr;
  UINT8 interface;
} ATM_CAPTURE_HDR, *pATM_CAPTURE_HDR;

typedef struct atmTxBufferHdr {
  ATM_VCC_DATA_PARMS dataParms;
  struct atmTxBufferHdr *next;
} ATM_TX_BUFFER_HDR, *pATM_TX_BUFFER_HDR;

typedef struct tx_buffer_list{
  pATM_TX_BUFFER_HDR headPtr;
  pATM_TX_BUFFER_HDR tailPtr;
  UINT32 len;
  UINT32 seqNumber;
  UINT32 lastSent;
  UINT32 sentInterval;
  UINT32 cellsPerPdu;
} ATM_TX_BUFFER_LIST, *pATM_TX_BUFFER_LIST;

typedef struct atmTestError {
  UINT32 total;
  UINT32 data_err;
  UINT32 data_length;
  UINT32 sequence_err;
  UINT32 aalCrcError;
  UINT32 aalCpcsLen0;
  UINT32 aalLenError;
  UINT32 aalSduLenError;
  UINT32 gfc;
  UINT32 crc;
  UINT32 pti;
  UINT32 pmi_2sml;
  UINT32 pmi_2big;
  UINT32 vcam_mme;
  UINT32 pne;
  UINT32 came_1;
  UINT32 came_0;
  UINT32 dc_1;
  UINT32 dc_0;
  UINT32 ec_1;
  UINT32 ec_0;
  UINT32 aal5_drop_cell;
  UINT32 routerDiscard_1;
  UINT32 routerDiscard_0;
  UINT32 camLkup;
  UINT32 idle;
  UINT32 hec;
} ATM_TEST_ERROR, *pATM_TEST_ERROR;

typedef struct atmMibStats {
  UINT32 tx_aal5_0;
  UINT32 tx_aal5_1;
  UINT32 tx_aal0_0;
  UINT32 tx_aal0_1;
  UINT32 rx_aal5_0;
  UINT32 rx_aal5_1;
  UINT32 rx_aal0_0;
  UINT32 rx_aal0_1;
} ATM_MIB_STATS, *pATM_MIB_STATS;

/* These are from TX status register; they are collected every 1 second interval */
typedef struct atmTxStats {
  UINT32 fifoFull;              /*  fifoFull_port0 */
  UINT32 aal2bigErr;      
  UINT32 aal5LenErr;  
  UINT32 aal5MaxLenErr;
  UINT32 droppedCellErr;        /* tx aal or tx atm dropped cell port 0 */
  UINT32 aal5PortNotEnableErr;  /* pne_err_port0 */
  UINT32 fifoFullErr;           /* ff_err_port0 */
  UINT32 aal5CountErr;
} ATM_TX_STATS, *pATM_TX_STATS;

/* these are from RX ATM and RX AAL status registers */
typedef struct atmRxStats {
  UINT32 gfcErr;
  UINT32 crcErr;
  UINT32 ptiErr;
  UINT32 vcamMmErr;          /* vcam_mme VCAM multiple match error */
  UINT32 camLookupErr;       /* came_port0 */
  UINT32 portNotEnableErr;   /* pne_err */
  UINT32 discardErr;         /* dc_port0 */
  UINT32 errCellErr;         /* ec_port0 */
  UINT32 routerDrop;         /* rxRouterStat_port0 */
  UINT32 aalDrop;            /* aal5d */
} ATM_RX_STATS, *pATM_RX_STATS;

typedef struct atmStats
{
  ATM_MIB_STATS mibStats;
  ATM_TX_STATS txStats;
  ATM_RX_STATS rxStats;
}ATM_STATS, *pATM_STATS;

typedef struct atm_test_tx_info {
  UINT32 index;
  UINT32 len;
  UINT32 lineTxInterval; 
  UINT32 count;
  UINT8  incremental;
  UINT8  dataByte;
  UINT8  aalType;
  UINT8  numOfQueues;
  UINT8  basePriority;
  UINT32 handle;
  UINT32 rate;
  UINT32 sending;
  UINT8  managementType; /* f4, f5, rm */
  UINT8  interleaveManagement;
  UINT16 managementVpi;
  UINT16 managementVci;
  UINT16 managementCrc;
  UINT32 managementInterface;
} ATM_TEST_TX_INFO, *pATM_TEST_TX_INFO;

typedef struct atm_test_info {
  ATM_TEST_TX_INFO atmTestTxInfo[UT_MAX_VCCS+1]; /* one extra for f4 since it doesn't
                                                    have a vcc created, index is last one */
  UT_TRAFFIC_DESCR_PARM_ENTRY ms_Tdt[UT_MAX_TD_INDEX];
  UT_VCC_CFG ms_VccCfgs[UT_MAX_VCCS];
  UT_VCC_ADDR_INFO ms_VccAddrs[UT_MAX_VCCS];
  UINT32 commandStatus; /* command-2 bytes, status 2 bytes */
  ATM_TEST_ERROR m_ucTestError[UT_MAX_VCCS+1];
  ATM_STATS atmStats;
  UINT8 displayData;  /* current mode: 0=disable, 1=enable */
  UINT8 captureData;  /* current mode: 0=disable, 1=enable */
  UINT8 verifyData;  /* current mode: 0=disable, 1=enable */
  UINT32 pduSent[UT_MAX_VCCS+1]; /* one extra for f4 */
  UINT32 pduReceived[UT_MAX_VCCS+1];
  UINT32 multiPriority;
} ATM_TEST_INFO, *pATM_TEST_INFO;

typedef struct atm_verfication_info {
  int seqNumber;
  UINT8 incremental;
  UINT8 dataByte;
  int len;
} ATM_VERIFICATION_INFO, *pATM_VERIFICATION_INFO;

typedef struct atm_data_struct {
  PATM_VCC_DATA_PARMS data;
  ATM_VCC_ADDR vccAddr;
} ATM_DATA_STRUCT, *PATM_DATA_STRUCT;

typedef struct atmDiagCb {
  ATM_TRAFFIC_DESCR_PARM_ENTRY ms_Tdt[UT_MAX_TD_INDEX];
  ATM_VCC_CFG ms_VccCfgs[UT_MAX_VCCS];
  UT_VCC_ADDR_INFO ms_VccAddrs[UT_MAX_VCCS];
  UINT32 ms_multiPriority[UT_MAX_VCCS];
  ATM_TX_BUFFER_LIST mTxHdrQ[UT_MAX_VCCS+1]; /* tx Q; an extra one for f4 cells */
  UT_VCC_LIST_INFO m_ulVccList[UT_MAX_VCCS+1]; /* tx Q; an extra one for f4 cells */
  UINT32 managementHandle_port0;
  UINT32 managementHandle_port1;
  UINT32 rxTaskId;  
  UINT32 txTaskId;
  UINT32 statsTaskId;
  UINT32 rxTaskSem;  /* protect Rx Q */
  UINT32 txTaskSem;  /* protect Tx Q */
  UINT32 rxQMuSem;   /* rx task semphore */
  UINT32 txQMuSem;   /* tx task semphore */
  UINT32 txTaskExit; /* clean up purpose */
  UINT32 rxTaskExit; /* clean up purpose */
  ATM_DATA_STRUCT m_pDpHead; /* rx Q */
  ATM_DATA_STRUCT m_pDpTail; /* rx Q */
  UINT8 displayData;  /* 1 to display rx data on screen; default is 0 */
  UINT8 captureData;
  UINT8 verifyData;
  ATM_CAPTURE_HDR m_ulData[UT_BUFFER_SIZE]; 
  int m_ulBufferPosition;
  UINT32 m_ulCurSeqNumber;
  ATM_TEST_ERROR m_ucTestError[UT_MAX_VCCS+1];
  ATM_STATS m_atmStats;
  ATM_VERIFICATION_INFO dataVerficationInfo[UT_MAX_VCCS]; 
  UINT8 txStop; 
} ATM_DIAG_CB, *pATM_DIAG_CB;

/* 0xfffe15c */
typedef union phyLastDescConfig { 
  struct {
    UINT32 unused:22;
    UINT32 rxCfg:2;
    UINT32 unused1:2;
    UINT32 txCfg:2;
    UINT32 numRxDesc:2;
    UINT32 numTxDesc:2;
  }bit;
  UINT32 reg;
} PHY_LAST_DESC_CONFIG, *pPHY_LAST_DESC_CONFIG;

/* 0xfffe4500-0xfffe45ff */
typedef union txAtmVpiVciTable { 
  struct {
    UINT32 unused:6;
    UINT32 swFlags:1;
    UINT32 crcEnable:1;
    UINT32 vpi: 8;
    UINT32 vci:16;
  }bit;
  UINT32 entry;
} TX_ATM_VPI_VCI_TABLE, *pTX_ATM_VPI_VCI_TABLE;

/* 0xfffe4600-0xfffe46ff */
typedef union RxAtmVpiVciTable {
  struct {
    UINT32 unused:6;
    UINT32 valid:1;
    UINT32 vpi:8;
    UINT32 vci:16;
    UINT32 port:1;
  } camSide;  /* even; */
  struct {
    UINT32 unused:21;
    UINT32 userDataIR:1; /* assert IR for user data immediate response */
    UINT32 oamIR:1; /* assert IR for OAM immediate response */
    UINT32 rmIR:1;  /* assert IR for RM immediate response */
    UINT32 vcId:3;  /* VCID */
    UINT32 userDataCrcEnable:1;
    UINT32 oamRouteCode:1;  /* 0=route to rx cell q; 1= route to rx mips cell fifo */
    UINT32 udrc:1;  /* User Data Routing Code */
    UINT32 circuitType:2; 
  } ramSide; /* odd; */
  UINT32 entry;
} RX_ATM_VPI_VCI_TABLE, *pRX_ATM_VPI_VCI_TABLE;

/* 6345; 0xfffe4300- 0xfffe43ff */
typedef union atmIntrRegs {
  struct {
    UINT32 unused:20; 
    UINT32 vcamMm:1;   /* RX VCAM multiple match */
    UINT32 rxRtDc:1;   /* Rx Router discard cell due to full rx buffer */
    UINT32 rpqIr:1;    /* Receive Packet Queue got a packet tagged with immediate response */
    UINT32 rcqIr:1;    /* Receive Cell Queue got a cell tagged with immediate response */
    UINT32 rpqWd:1;    /* RX Pkt Q watchdog- no pkt rxed for the duration defined in RCQ wd timer */
    UINT32 rcqWd:1;    /* RX Cell Q watchdog */
    UINT32 mibHf:1;    /* one or more of the MIB coutners is half full */
    UINT32 fpqAe:1;    /* Free Packet Queue almost empty- has fewer buffers than FPQ watermark */
    UINT32 rpqAf:1;    /* Rx Packet Queue has exceeded RPQ watermark */
    UINT32 fcqAe:1;    /* Free Cell Queue almost Empty */
    UINT32 rcqAf:1;    /* Rx Cell Q almost full */
    UINT32 txs:1;      /* Tx SDRAM Interrupt- one of the TX SDRAM sub-channels intr is set */
  }statusMaskBit;      /* status & interrupt mask */
  struct {
    UINT32 fcqAeWm:16;  /* Free Cell Q almost empty watermark */
    UINT32 rcqAfWm:16;  /* Rx Cell Q almost full watermark */
  }rxCellQBit;
  struct {
    UINT32 fpqAeWm:16;  /* Free Packet Q almost empty watermark */
    UINT32 rpqAfWm:16;  /* Rx Paket Q almost full watermark */
  }rxPktQBit;
  struct {
    UINT32 pktWdTo:16;  /* Watchdog timeout value in 50 uSec increments */
    UINT32 cellWdTo:16; /* Watchdog timeout value in 50 uSec increments */
  }rxWdTimer;
} ATM_INTR_REGS, *pATM_INTR_REGS;

/* 0xfffe4700-0xfffe47ff */
typedef union atmShaperCtrlReg {
  UINT32 entry;
} ATM_SHAPER_CTRL_REG, *pATM_SHAPER_CTRL_REG;

typedef union atmShaperVbrReg {
  struct {
    UINT32 unused:1;
    UINT32 bt:19;
    UINT32 scr:12;
  }bit;
  UINT32 entry;
} ATM_SHAPER_VBR_REG, *pATM_SHAPER_VBR_REG;

typedef union atmCellHdr {
  struct {
    UINT32 gfc:4;  
    UINT32 msb_vpi:4;  
    UINT32 vpi:4;  
    UINT32 msb_vci:4;  
    UINT32 vci:8;
    UINT32 lsb_vci:4;  
    UINT32 pt:3;  
    UINT32 clp:1;  
  }bit;
  UINT32 word1;
} ATM_CELL_HDR, *pATM_CELL_HDR;
#define ATM_RX_AAL_STATUS_ERROR_MASK_PORT0 0x108

/* 0xfffe4428 */
typedef union atmRxAalStatusReg {
  struct {
    UINT32 unused:22;  
    UINT32 rxRouterStat_port1:1; /* RX cells dropped due to full cell buffer; */
    UINT32 rxRouterStat_port0:1; /* bit 8=port 0 fifo rx drop cell */
    UINT32 aal0ccnt_port1:1;     /* aal0 cell count has been incremented; bit 4=port0 */
    UINT32 aal0ccnt_port0:1;     /* aal0 cell count has been incremented; bit 4=port0 */
    UINT32 aal5ccnt_port1:1;     /* aal5 cell count has been incremented; bit 4=port0 */
    UINT32 aal5ccnt_port0:1;     /* aal5 cell count has been incremented; bit 4=port0 */
    UINT32 aal5d:1;        /* aal5 dropped cells */
    UINT32 aal5p:1;        /* aal5 pdu received */
    UINT32 aalxp:1;        /* non aal5 received */
    UINT32 aal5c:1;        /* aal5 received cells */
  }bit;
  UINT32 reg;
} ATM_RX_AAL_STATUS_REG, *pATM_RX_AAL_STATUS_REG;
/*  0xfffe441c */
#define ATM_RX_STATUS_ERROR_MASK_PORT0 0x32354
typedef union atmRxStatusReg {
  struct {
    UINT32 unused:14;  
    UINT32 gfc_err:1;  /* non zero gfc detected */
    UINT32 crc_err:1;  /* CRC-10 error detected on OAM/RM cells */
    UINT32 idle_err:1; /* Idle cell detected */
    UINT32 pti_err:1;  /* PTI Error detected (i.e. PT=binary 111) */
    UINT32 vcam_mme:1; /* VCAM multiple match error */
    UINT32 pne_err:1;  /* port not enable error */
    UINT32 came_port1:1;     /* PER port cam lookup error; bit6=port 0  */
    UINT32 came_port0:1;     /* PER port cam lookup error; bit6=port 0  */
    UINT32 dc_port1:1;       /* per port dropped cell; bit 4= port 0 */  
    UINT32 dc_port0:1;       /* per port dropped cell; bit 4= port 0 */  
    UINT32 ec_port1:1;       /* per port erred cell; bit 2=port 0 */
    UINT32 ec_port0:1;       /* per port erred cell; bit 2=port 0 */
    UINT32 vc_port1:1;       /* per port valid cell; bit 0=port 0 */
    UINT32 vc_port0:1;       /* per port valid cell; bit 0=port 0 */
  }bit;
  UINT32 reg;
} ATM_RX_STATUS_REG, *pATM_RX_STATUS_REG;

#define ATM_TX_STATUS_ERROR_MASK_PORT0  0x41e80c54
typedef union atmTxStatusReg {
  struct {
    UINT32 fifoFull_port1:1; /* per port FIFO Full Status (1=full) */
    UINT32 fifoFull_port0:1; /* per port FIFO Full Status (1=full) */
    UINT32 unused:1;  
    UINT32 aal0_port1:1; /* aal0_port1 tx */
    UINT32 aal0_port0:1; /* aal0_port0 tx */
    UINT32 aal5_port1:1; /* aal5_port1 tx */
    UINT32 aal5_port0:1; /* aal5_port0 tx */
    UINT32 aal2big:1;  /* aal too big cell input */
    UINT32 aal5liErr:1;/* aal5 length indicator error */
    UINT32 aal5mlErr:1;/* aal5 max length error */
    UINT32 aal5ctErr:1;/* aal5 count error */
    UINT32 unused1:1;  
    UINT32 aal5d:1;    /* aal5 drop cell */
    UINT32 aal5p:1;    /* aal5 pdu passed */
    UINT32 aalxc:1;    /* non aal5 cell passed */
    UINT32 aal5c:1;    /* aal cell passed */
    UINT32 dropCell_port1:1; /* tx aal or tx atm dropped cell */
    UINT32 dropReq_port1:1;  /* one of the port dropped request */
    UINT32 scheCell_port1:1; /* per port scheduled cell */
    UINT32 sit_port1:1;      /* per port schedule interval timer count event */
    UINT32 dropCell_port0:1; /* tx aal or tx atm dropped cell */
    UINT32 dropReq_port0:1;  /* one of the port dropped request */
    UINT32 scheCell_port0:1; /* per port scheduled cell */
    UINT32 sit_port0:1;      /* per port schedule interval timer count event */
    UINT32 pne_err_port1:1;  /* port not enable error */
    UINT32 pne_err_port0:1;  /* port not enable error */
    UINT32 ff_err_port1:1;   /* fifo full error */
    UINT32 ff_err_port0:1;   /* fifo full error */
    UINT32 dc_port1:1;       /* per port dropped cell */
    UINT32 dc_port0:1;       /* per port dropped cell */
    UINT32 pc_port1:1;       /* per port processed cell */
    UINT32 pc_port0:1;       /* per port processed cell */
  }bit;
  UINT32 reg;
} ATM_TX_STATUS_REG, *pATM_TX_STATUS_REG;


typedef union atmTxHdrReg {
  struct {
    UINT32 unused1:14;
    UINT32 aal5SwTrailer:1; /* software trailer enable */
    UINT32 schedCrst_1:1;  /* scheuler reset */
    UINT32 schedCrst_0:1;  
    UINT32 unused:1; 
    UINT32 haltShpt_1:1;   /* halt shaper, used for dynamic configuration of shaper */
    UINT32 haltShpt_0:1;
    UINT32 altGFC:4;       /* alternate GFC value */
    UINT32 altGFCen_1:1;
    UINT32 altGFCen_0:1;   /* alternate GFC mode enable */
    UINT32 fRst_1:1;
    UINT32 fRst_0:1;
    UINT32 oamCrcEn_1:1;
    UINT32 oamCrcEn_0:1;
    UINT32 txEn_1:1;
    UINT32 txEn_0:1;
  }bit;
  UINT32 reg;
} ATM_TX_HDR_CFG_REG, *pATM_TX_HDR_CFG_REG;

typedef union rxAalError {
  struct {
    UINT8 crc:1;             /* aal5 CRC error */
    UINT8 cpcsLen0:1;        /* aal5 cpcsLen error */
    UINT8 length:1;          /* aal5 len error */
    UINT8 maxSduExceed:1;    /* max sdu exceed error */
    UINT8 unused:4;
  }bit;
  UINT8 entry;
} RX_AAL_ERROR;

typedef union rxAtmError {
  struct {
    UINT8 pne:1;             /* port not enable error */
    UINT8 hec:1;             /* HEC error */
    UINT8 pti:1;             /* pti error */
    UINT8 idle:1;            /* idle rx */
    UINT8 camLkup:1;         /* cam look up error */
    UINT8 unused:1;          
    UINT8 oamCrc:1;          /* oam crc */
    UINT8 gfc:1;             /* gfc error */
  }bit;
  UINT8 entry;
} RX_ATM_ERROR;

/* 0xfffe442c */
typedef union atmUtopiaCfg {
  struct {
    UINT32 unused:26;
    UINT32 rxLevel2:1;   /* when set=level 2, when 0=level 1 */
    UINT32 rxEn:1;       /* enable RX Utopia Operation */
    UINT32 unused1:2;
    UINT32 txLevel2:1;   /* when set=level 2, when 0=level 1 */
    UINT32 txEn:1;       /* enable TX Utopia Operation */
  }bit;
  UINT32 entry;
} ATM_UTOPIA_CFG, *pATM_UTOPIA_CFG;

typedef union portSchedulerCfg {
  struct {
    UINT32 cit:16;
    UINT32 unused:12;
    UINT32 mode:2;
    UINT32 arb:1;
    UINT32 en:1;
  }bit;
  UINT32 entry;
} ATM_PORT_SCHEDULER_CFG, *pATM_PORT_SCHEDULER_CFG;

/* memory map operation definition */
typedef struct atm_regs {
    int kmem_fd;
    char *mmap_addr;
    unsigned long addr;
    unsigned int size;
    unsigned int offset;
} atm_regs;

int getVccNextIndex(void);
void removeVccIndex(int index);
int isVpiVciExisted(UINT32 interface, UINT16 vpi, UINT16 vci);
void atmDiagInit(void);
BCMATM_STATUS bcmAtmDiagInit(void);
BCMATM_STATUS bcmAtmDiagUnInit(void);
BCMATM_STATUS bcmAtmAddVccCommand(pUT_VCC_ADDR_INFO pVccAddrs, pUT_VCC_CFG pVccCfg,
                                  pUT_TRAFFIC_DESCR_PARM_ENTRY pTd);
BCMATM_STATUS bcmAtmSendVccCommand(pATM_TEST_TX_INFO pAtmInfo);
BCMATM_STATUS bcmAtmCaptureCommand(int mode);
BCMATM_STATUS bcmAtmSendManagementCommand(pATM_TEST_TX_INFO pAtmInfo);
BCMATM_STATUS bcmAtmDeleteVccCommand(pUT_VCC_ADDR_INFO pVccAddrs);
BCMATM_STATUS bcmAtmSendAllVccsCommand(pATM_TEST_TX_INFO pAtmInfo);
BCMATM_STATUS bcmAtmSendMultiPriorityCommand(pATM_TEST_TX_INFO pAtmInfo);
BCMATM_STATUS bcmAtmSendLoopbackCommand(UINT8 mode);
BCMATM_STATUS bcmAtmAddMPVccCommand(pUT_VCC_ADDR_INFO pVccAddrs, pUT_VCC_CFG pVccCfg,
                                    pUT_TRAFFIC_DESCR_PARM_ENTRY pTd);
BCMATM_STATUS bcmAtmDeleteMPVccCommand(pUT_VCC_ADDR_INFO pVccAddrs);
BCMATM_STATUS bcmAtmSendMPVccCommand(pATM_TEST_TX_INFO pAtmInfo);
BCMATM_STATUS bcmAtmSendMultiPriorityCommand(pATM_TEST_TX_INFO pAtmInfo);
BCMATM_STATUS bcmAtmModifyTDCommand(pUT_TRAFFIC_DESCR_PARM_ENTRY pTD, UINT32 index);
int bcmAtmGetStatsCommand(int reset);
int bcmAtmToggleVerifyCommand(void);
int bcmAtmToggleCaptureCommand(void);
int bcmAtmToggleDisplayCommand(void);
int bcmAtmStopTxCommand(void);
int isVpiVciExisted(UINT32 interface, UINT16 vpi, UINT16 vci);
int bcmDiag_unmapregs(atm_regs *mapregs);
atm_regs *bcmDiag_mapregs(unsigned long addr, int size);
int bcmDiagGetVerificationStats(int vcc,char *pResult);
void bcmDiagClearSARstats(void);
void bcmDiagReadSARstats(int parm);
int bcmDiagGetSARStats(char *pResult);
#if defined(__cplusplus)
}
#endif

#endif
