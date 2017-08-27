/*
<:copyright-BRCM:2012:DUAL/GPL:standard 

   Copyright (c) 2012 Broadcom 
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

#ifndef __BCM60333_MAP_PART_H
#define __BCM60333_MAP_PART_H

#ifdef __cplusplus
extern "C" {
#endif

#define CHIP_FAMILY_ID_HEX  0x60333

#ifndef __BCM60333_MAP_H
#define __BCM60333_MAP_H

#include "bcmtypes.h"

#define SDRAM_CTRL_BASE             0xb2000000  /* SDRAM Control */
#define ENET_CORE0_BASE             0xb2400000
#define ENET_CORE1_BASE             0xb2600000
#define PCIE_BASE                   0xb2a00000  /* PCIE Control */

#define BRIDGE_BASE                 0xb3000000
#define PERF_BASE                   0xb3e00000  /* chip control */
#define TIMR_BASE                   0xb3e00100  /* timer registers */
#define GPIO_BASE                   0xb3e00200  /* gpio registers */
#define MISC_BASE                   0xb3e00300  /* Miscellaneous registers */
#define STRAP_BASE                  0xb3e00304  /* Strap Override Control Registers */
#define BSTI_BASE                   0xb3e003f8  /* Serial Interface Control registers */
#define OTP_BASE                    0xb3e00400  /* OTP Controller registers */
#define UART_BASE                   0xb3e00500  /* UART registers */
#define PERIPH_MUTEX_BASE           0xb3e00700  /* Periph Mutex registers */
#define HSSPIM_BASE                 0xb3e01000  /* High-Speed SPI registers */

#define ETH0_DMA_BASE               0xb2410000
#define ETH1_DMA_BASE               0xb2610000
#define BRIDGE_DMA_BASE             0xb3020000

#define SWITCH_DMA_BASE             0           /* Dummy, to avoid compiler errors */

#endif


#define  DUMMY_A_BASE               0x82100000    /* PLCPHY DUMMY_A: QBUS dummy registers (segment A) */
#define  DUMMY_B_BASE               0x82200000    /* PLCPHY DUMMY_B: QBUS dummy registers (segment B) */
#define  DUMMY_C_BASE               0x82300000    /* PLCPHY DUMMY_C: QBUS dummy registers (segment C) */
#define  DUMMY_D_BASE               0x82400000    /* PLCPHY DUMMY_D; QBUS dummy registers (segment D) */

/*
 * 0x82100000..0x8210004c    DUMMY_A_RW_DUMMY_REG[0..19]    "Read/Write Dummy Register 0..19"
 * 0x82100050..0x8210006c    DUMMY_A_R_DUMMY_REG[0..7]      "Read Dummy Register 0..7"
 * 0x82100070..0x8210007c    DUMMY_A_W_DUMMY_REG[0..3]      "Write Dummy Register 0..3"
 */

/* Use DUMMY_A_RW:19 to send QoS-related data to plc-phy */
#define PLC_PHY_AC_CONTROL_REGISTER DUMMY_A_BASE + (19 * sizeof(int))

/* PLC status register address */
#define PLC_STATUS_ADDR                      0x82100014
#define PLC_STATUS_BOOTING                   0
#define PLC_STATUS_RUNNING                   1
#define PLC_STATUS_ERROR                     2
#define PLC_STATUS_RUNNING_WDOG_DISABLED     3
#define PLC_STATUS_COREDUMP_REQUESTED        0xC08EC08E

/* ROM-based Chip identification:
 * ROM address 0x40000 (0x80040000) contains 0xFFFFFFFF in A0
 *                                           0xB6FF89FF in B0
 */
#define PLC_ROM_CHECK_ADDR          0x80040000
#define PLC_ROM_ID_B0               0xB6FF89FF
#define PLC_ROM_ID_A0               0xFFFFFFFF


/*
** SDR/DDR Memory Controller Register Set Definitions.
*/

typedef struct MEMCControl {
    uint32  SDR_CFG;                             /* 0x00 */
#define MEMC_SDRAM_SPACE_SHIFT         4
#define MEMC_SDRAM_SPACE_MASK          (0xF<<MEMC_SDRAM_SPACE_SHIFT)
#define MEMC_SDRAM_SPACE2_SHIFT        8
#define MEMC_SDRAM_SPACE2_MASK         (0xF<<MEMC_SDRAM_SPACE2_SHIFT)
    uint32  SEMAPHORE0;                          /* 0x04 */
    uint32  SEMAPHORE1;                          /* 0x08 */
    uint32  SEMAPHORE2;                          /* 0x0C */
    uint32  SEMAPHORE3;                          /* 0x10 */
    uint32  PRI_CFG;                             /* 0x14 */
    uint32  PID_SELECT0;                         /* 0x18 */
    uint32  PID_SELECT1;                         /* 0x1C */
    uint32  UBUS2_THRESHOLD;                     /* 0x20 */
    uint32  AUTO_REFRESH;                        /* 0x24 */
    uint32  TIMING_PARAM;                        /* 0x28 */
    uint32  RESERVED1;                           /* 0x2C */
    uint32  DDR_TIMING_PARAM;                    /* 0x30 */
    uint32  DDR_DRIVE_PARAM;                     /* 0x34 */
} MEMCControl;

#define MEMC ((volatile MEMCControl * const) SDRAM_CTRL_BASE)



/*
** Ethernet Subsystem
*/

typedef struct {
    uint32 unused1;                     /* 0x00 */
    uint32 hdBkpCntl;                   /* 0x04 */
#define UNIMAC_CTRL_HD_FC_ENA           1
#define UNIMAC_CTRL_HD_FC_BKOFF_OK      (1 << 1)
#define UNIMAC_CTRL_IPG_CONFIG_RX       (0x1f << 2)

    uint32 cmd;                         /* 0x08 */
#define UNIMAC_CTRL_TX_ENA              1
#define UNIMAC_CTRL_RX_ENA              (1 << 1)
#define UNIMAC_CTRL_ETH_SPEED           (0x3 << 2)
#define UNIMAC_CTRL_ETH_SPEED_SHIFT     2
#define UNIMAC_CTRL_PROMIS_EN           (1 << 4)
#define UNIMAC_CTRL_PAD_EN              (1 << 5)
#define UNIMAC_CTRL_CRC_FWD             (1 << 6)
#define UNIMAC_CTRL_PAUSE_FWD           (1 << 7)
#define UNIMAC_CTRL_RX_PAUSE_IGN        (1 << 8)
#define UNIMAC_CTRL_TX_ADDR_INS         (1 << 9)
#define UNIMAC_CTRL_HD_ENA              (1 << 10)
#define UNIMAC_CTRL_OVERFLOW_EN         (1 << 12)
#define UNIMAC_CTRL_SW_RESET            (1 << 13)
#define UNIMAC_CTRL_LCL_LOOP_EN         (1 << 15)
#define UNIMAC_CTRL_ENA_EXT_CFG         (1 << 22)
#define UNIMAC_CTRL_CNTL_FRM_EN         (1 << 23)
#define UNIMAC_CTRL_NO_LGTH_CHK         (1 << 24)
#define UNIMAC_CTRL_RMT_LOOP_EN         (1 << 25)
#define UNIMAC_CTRL_RX_ERR_DISC         (1 << 26)
#define UNIMAC_CTRL_PRBL_EN             (1 << 27)
#define UNIMAC_CTRL_TX_PAUSE_IGN        (1 << 28)
#define UNIMAC_CTRL_TXRX_EN_CFG         (1 << 29)
#define UNIMAC_CTRL_RUNT_FILTER_DIS     (1 << 30)

    uint32 mac0;                        /* 0x0c */
    uint32 mac1;                        /* 0x10 */
#define UNIMAC_CTRL_MAC1                0xffff

    uint32 frmLen;                      /* 0x14 */
#define UNIMAC_CTRL_FRAME_LEN           0x3fff

    uint32 pauseQuant;                  /* 0x18 */
#define UNIMAC_CTRL_PAUSE_QUANT         0xffff

    uint32 unused2[8];
    uint32 txTsSeqId;                   /* 0x3c */
#define UNIMAC_CTRL_TXTS_SEQ_ID         0xffff
#define UNIMAC_CTRL_TXTS_VALID          (1 << 16)

    uint32 unused3;
    uint32 mode;                        /* 0x44 */
#define UNIMAC_CTRL_MAC_SPEED           0x3
#define UNIMAC_CTRL_MAC_DUPLEX          (1 << 2)
#define UNIMAC_CTRL_MAC_RX_PAUSE        (1 << 3)
#define UNIMAC_CTRL_MAC_TX_PAUSE        (1 << 4)
#define UNIMAC_CTRL_MAC_LINK_STAT       (1 << 5)

    uint32 frmTag0;                     /* 0x48 */
#define UNIMAC_CTRL_OUTER_TAG           0xffff
#define UNIMAC_CTRL_OUTER_TPID_EN       (1 << 16)

    uint32 frmTag1;                     /* 0x4c */
#define UNIMAC_CTRL_INNER_TAG           0xffff

    uint32 unused4[3];
    uint32 txIpgLen;                    /* 0x5c */
#define UNIMAC_CTRL_TX_IPG_LEN          0x1f

    uint32 unused5;
    uint32 eeeCtrl;                     /* 0x64 */
#define UNIMAC_CTRL_EEE_EN             (1 << 3)
#define UNIMAC_CTRL_RX_FIFO_CHK        (1 << 4)
#define UNIMAC_CTRL_EEE_TXCLK_DIS      (1 << 5)
#define UNIMAC_CTRL_DIS_EEE_10M        (1 << 6)
#define UNIMAC_CTRL_LP_IDLE_PRED_MODE  (1 << 7)

    uint32 miiEeeLp1Timer;              /* 0x68 */
    uint32 gmiiEeeLp1Timer;             /* 0x6c */
    uint32 eeeRefCount;                 /* 0x70 */
#define UNIMAC_CTRL_EEE_REF_COUNT       0xffff

    uint32 unused6;
    uint32 rxPktDropStatus;             /* 0x78 */
#define UNIMAC_CTRL_RX_IPG_INV          1

    uint32 symmetricIdleThreshold;      /* 0x7c */
#define UNIMAC_CTRL_THRESHOLD_VAL       0xffff

    uint32 miiEeeWakeTimer;             /* 0x80 */
#define UNIMAC_CTRL_EEE_WAKE_TIMER      0xffff

    uint32 gmiiEeeWakeTimer;            /* 0x84 */
    uint32 umacRevId;                   /* 0x88 */
#define UNIMAC_CTRL_PATCH               0xff
#define UNIMAC_CTRL_REV_ID_MIN          (0xff << 8)
#define UNIMAC_CTRL_REV_ID_MAJ          (0xff << 16)

    uint32 unused7[33];
    uint32 macsecProgTxCrc;             /* 0x110 */
    uint32 macsecCntrl;                 /* 0x114 */
#define UNIMAC_CTRL_TX_LAUNCH_EN        1
#define UNIMAC_CTRL_TX_CRC_CORRUPT_EN   (1 << 1)
#define UNIMAC_CTRL_TX_CRC_PROGRAM      (1 << 2)

    uint32 txStatusCntrl;               /* 0x118 */
#define UNIMAC_CTRL_TX_TS_FIFO_FULL     1
#define UNIMAC_CTRL_TX_TS_FIFO_EMPTY    (1 << 1)
#define UNIMAC_CTRL_WORD_AVAIL          (0x7 << 2)

    uint32 txTsData;                    /* 0x11c */
    uint32 unused8[4];
    uint32 pauseCntrl;                  /* 0x130 */
#define UNIMAC_CTRL_PAUSE_TIMER         0x1ffff
#define UNIMAC_CTRL_PAUSE_CTRL_EN       (1 << 17)

    uint32 txFifoFlush;                 /* 0x134 */
#define UNIMAC_CTRL_TX_FLUSH            1

    uint32 rxFifoStat;                  /* 0x138 */
#define UNIMAC_CTRL_RXFIFO_STATUS       0x3

    uint32 txFifoStat;                  /* 0x13c */
#define UNIMAC_CTRL_TXFIFO_UNDERRUN     1
#define UNIMAC_CTRL_TXFIFO_OVERRUN      (1 << 1)

    uint32 pppCntrl;                    /* 0x140 */
#define UNIMAC_CTRL_PPP_EN_TX           1
#define UNIMAC_CTRL_PPP_EN_RX           (1 << 1)
#define UNIMAC_CTRL_FORCE_PPP_XON       (1 << 2)
#define UNIMAC_CTRL_RX_PASS_PFC_FRM     (1 << 4)
#define UNIMAC_CTRL_PFC_STATS_EN        (1 << 5)

    uint32 pppRefreshCntrl;             /* 0x144 */
#define UNIMAC_CTRL_PPP_REFRESH_EN      1
#define UNIMAC_CTRL_PPP_REFRESH_TIMER   (0xffff << 16)
} EnetCoreUnimac;


typedef struct {
    uint32 gr64;        /* 0x00 */
    uint32 gr127;       /* 0x04 */
    uint32 gr255;       /* 0x08 */
    uint32 gr511;       /* 0x0c */
    uint32 gr1023;      /* 0x10 */
    uint32 gr1518;      /* 0x14 */
    uint32 grmgv;       /* 0x18 */
    uint32 gr2047;      /* 0x1c */
    uint32 gr4095;      /* 0x20 */
    uint32 gr9216;      /* 0x24 */
    uint32 grpkt;       /* 0x28 */
    uint32 grbyt;       /* 0x2c */
    uint32 grmca;       /* 0x30 */
    uint32 grbca;       /* 0x34 */
    uint32 grfcs;       /* 0x38 */
    uint32 grxcf;       /* 0x3c */
    uint32 grxpf;       /* 0x40 */
    uint32 grxuo;       /* 0x44 */
    uint32 graln;       /* 0x48 */
    uint32 grflr;       /* 0x4c */
    uint32 grcde;       /* 0x50 */
    uint32 grfcr;       /* 0x54 */
    uint32 grovr;       /* 0x58 */
    uint32 grjbr;       /* 0x5c */
    uint32 grmtue;      /* 0x60 */
    uint32 grpok;       /* 0x64 */
    uint32 gruc;        /* 0x68 */
    uint32 grppp;       /* 0x6c */
    uint32 grcrc;       /* 0x70 */
    uint32 unused1[3];
    uint32 tr64;        /* 0x80 */
    uint32 tr127;       /* 0x84 */
    uint32 tr255;       /* 0x88 */
    uint32 tr511;       /* 0x8c */
    uint32 tr1023;      /* 0x90 */
    uint32 tr1518;      /* 0x94 */
    uint32 trmgv;       /* 0x98 */
    uint32 tr2047;      /* 0x9c */
    uint32 tr4095;      /* 0xa0 */
    uint32 tr9216;      /* 0xa4 */
    uint32 gtpkt;       /* 0xa8 */
    uint32 gtmca;       /* 0xac */
    uint32 gtbca;       /* 0xb0 */
    uint32 gtxpf;       /* 0xb4 */
    uint32 gtxcf;       /* 0xb8 */
    uint32 gtfcs;       /* 0xbc */
    uint32 gtovr;       /* 0xc0 */
    uint32 gtdrf;       /* 0xc4 */
    uint32 gtedf;       /* 0xc8 */
    uint32 gtscl;       /* 0xcc */
    uint32 gtmcl;       /* 0xd0 */
    uint32 gtlcl;       /* 0xd4 */
    uint32 gtxcl;       /* 0xd8 */
    uint32 gtfrg;       /* 0xdc */
    uint32 gtncl;       /* 0xe0 */
    uint32 gtjbr;       /* 0xe4 */
    uint32 gtbyt;       /* 0xe8 */
    uint32 gtpok;       /* 0xec */
    uint32 gtuc;        /* 0xf0 */
    uint32 unused2[3];
    uint32 rrpkt;       /* 0x100 */
    uint32 rrund;       /* 0x104 */
    uint32 rrfrg;       /* 0x108 */
    uint32 rrbyt;       /* 0x10c */
    uint32 unused3[24];
    uint32 mibCntrl;    /* 0x180 */
#define MIB_CNTRL_RX_CNT_RST     1
#define MIB_CNTRL_RUNT_CNT_RST   (1 << 1)
#define MIB_CNTRL_TX_CNT_RST     (1 << 2)
} EnetCoreMib;


typedef struct {
    uint32 bkpuCntrl;           /* 0x00 */
#define IF_CNTRL_BACKPRESS_ON    1

    uint32 control;             /* 0x04 */
#define IF_CNTRL_DIRECT_GMII           1
#define IF_CNTRL_SS_MODE_MII           (1 << 2)
#define IF_CNTRL_SS_MODE_MII_SHIFT     2
#define IF_CNTRL_RGMII_TX_DELAY        (1 << 3)
#define IF_CNTRL_RGMII_TX_DELAY_SHIFT  3
#define IF_CNTRL_XMII_IF_MODE          (0x3 << 4)
#define IF_CNTRL_XMII_IF_MODE_SHIFT    4

    uint32 eeeIfStatus;         /* 0x08 */
#define IF_CNTRL_LPI_RX_DETECT   1
#define IF_CNTRL_LPI_TX_DETECT   (1 << 1)

    uint32 burstLen;            /* 0x0c */
#define IF_CNTRL_BURST_LEN       0xff

    uint32 rxErrMask;           /* 0x10 */
#define IF_CNTRL_PACKET_SKIPPED            1
#define IF_CNTRL_STACK_VLAN                (1 << 1)
#define IF_CNTRL_CARRIER_EVENT             (1 << 2)
#define IF_CNTRL_RX_ERROR                  (1 << 3)
#define IF_CNTRL_CRC_ERROR                 (1 << 4)
#define IF_CNTRL_WRONG_FRAME_LEN           (1 << 5)
#define IF_CNTRL_FRAME_LEN_OUT_OF_RANGE    (1 << 6)
#define IF_CNTRL_GOOD_PACKET               (1 << 7)
#define IF_CNTRL_MCAST_DETECT              (1 << 8)
#define IF_CNTRL_BCAST_DETECT              (1 << 9)
#define IF_CNTRL_DRIBBLE_NIBBLE            (1 << 10)
#define IF_CNTRL_CONTROL_FRAME             (1 << 11)
#define IF_CNTRL_PAUSE_FRAME               (1 << 12)
#define IF_CNTRL_UNSUPPORTED_FRAME         (1 << 13)
#define IF_CNTRL_VLAN_TAG                  (1 << 14)
#define IF_CNTRL_UCAST_FRAME               (1 << 15)
#define IF_CNTRL_TRUNCATED_FRAME           (1 << 16)
#define IF_CNTRL_RUNT_PACKET               (1 << 17)

    uint32 txActivityLedOn;     /* 0x14 */
    uint32 txActivityLedOff;    /* 0x18 */
    uint32 rxActivityLedOn;     /* 0x1c */
    uint32 rxActivityLedOff;    /* 0x20 */
    uint32 rxRgmiiIdKey;        /* 0x24 */
#define IF_CNTRL_RX_RGMII_ID_KEY           0xffff

    uint32 coreIrqEnable;       /* 0x28 */
#define IF_CNTRL_RX_RUNT                       1
#define IF_CNTRL_RX_FRAME_TRUNC                (1 << 1)
#define IF_CNTRL_RX_PAUSE_FRAME                (1 << 2)
#define IF_CNTRL_RX_BAD_LENGTH                 (1 << 3)
#define IF_CNTRL_RX_CRC_ERR                    (1 << 4)
#define IF_CNTRL_RX_ERR                        (1 << 5)
#define IF_CNTRL_EEE_LPI_RX_DETECT_ASSERTED    (1 << 6)
#define IF_CNTRL_EEE_LPI_RX_DETECT_DEASSERTED  (1 << 7)
#define IF_CNTRL_RX_PACKET_DROPPED             (1 << 8)
#define IF_CNTRL_IUDMA_FLOWCTRL_ASSERTED       (1 << 9)
#define IF_CNTRL_IUDMA_FLOWCTRL_DEASSERTED     (1 << 10)
#define IF_CNTRL_NTC_2ND_COUNTER_WRAP          (1 << 11)
#define IF_CNTRL_TX_PAUSE_FRAME                (1 << 16)
#define IF_CNTRL_TX_FIFO_UNDERRUN              (1 << 17)
#define IF_CNTRL_TX_JUMBO                      (1 << 18)
#define IF_CNTRL_TX_ERR                        (1 << 19)
#define IF_CNTRL_EEE_LPI_TX_DETECT_ASSERTED    (1 << 20)
#define IF_CNTRL_EEE_LPI_TX_DETECT_DEASSERTED  (1 << 21)
#define IF_CNTRL_HD_PKT_DEFERRED               (1 << 22)
#define IF_CNTRL_HD_LATE_COLLISION             (1 << 23)
#define IF_CNTRL_HD_MAX_RETX_LIMIT_REACHED     (1 << 24)
#define IF_CNTRL_HD_EXCESS_DEFERRAL            (1 << 25)
#define IF_CNTRL_MAC_IN_PAUSE_ASSERTED         (1 << 26)
#define IF_CNTRL_MAC_IN_PAUSE_DEASSERTED       (1 << 27)

    uint32 coreIrqRead;         /* 0x2c */
    uint32 txBufFillThreshold;  /* 0x30 */
#define IF_CNTRL_TX_BUF_FILL_THRESHOLD         0xff

    uint32 timestampEnable;     /* 0x34 */
#define IF_CNTRL_TIMESTAMP_ENABLE              1
    uint32 timestampValue;      /* 0x38 */
    uint32 timestampOffset;     /* 0x3c */
    uint32 timestampFdOffset;   /* 0x40 */
#define IF_CNTRL_TIMESTAMP_FD_OFFSET           0x7fffffff
#define IF_CNTRL_TIMESTAMP_FD_SIGN             (1 << 31)

    uint32 eeeLpiExtInd;        /* 0x44 */
#define IF_CNTRL_EEE_LPI_EXT_IND               1

    uint32 rxBufXonThreshold;   /* 0x48 */
#define IF_CNTRL_RX_BUFFER_XON_THRESHOLD       0x7ff

    uint32 rxBufXoffThreshold;  /* 0x4c */
#define IF_CNTRL_RX_BUFFER_XOFF_THRESHOLD      0x7ff
#define IF_CNTRL_RX_BUFFER_XOFF_THRESHOLD_EN   (1 << 11)
} EnetCoreIf;


typedef struct {
    uint32 mdioControl;         /* 0x00 */
#define MDIO_CNTRL_MDC_CLK_RATE          0x7f
#define MDIO_CNTRL_ENABLE_MDIO_PREAMBLE_SHIFT   7
#define MDIO_CNTRL_ENABLE_MDIO_PREAMBLE_MASK    (1 << MDIO_CNTRL_ENABLE_MDIO_PREAMBLE_SHIFT)

    uint32 mdioWrite;           /* 0x04 */
#define MDIO_CNTRL_MDIO_DATA             0xffff
#define MDIO_CNTRL_TURN_AROUND_SHIFT     16
#define MDIO_CNTRL_TURN_AROUND_MASK      (0x3 << MDIO_CNTRL_TURN_AROUND_SHIFT)
#define MDIO_CNTRL_REGISTER_ADDR_SHIFT   18
#define MDIO_CNTRL_REGISTER_ADDR_MASK    (0x1f << MDIO_CNTRL_REGISTER_ADDR_SHIFT)
#define MDIO_CNTRL_PMD_SHIFT             23
#define MDIO_CNTRL_PMD_MASK              (0x1f << MDIO_CNTRL_PMD_SHIFT)
#define MDIO_CNTRL_OPCODE_SHIFT          28
#define MDIO_CNTRL_OPCODE_MASK           (0x3 << MDIO_CNTRL_OPCODE_SHIFT)
#define MDIO_CNTRL_MDIO_START_BITS_SHIFT 30
#define MDIO_CNTRL_MDIO_START_BITS_MASK  (0x3 << MDIO_CNTRL_MDIO_START_BITS_SHIFT)

    uint32 mdioRead;            /* 0x08 */
#define MDIO_CNTRL_READ                  0xffff

    uint32 mdioStatus;           /* 0x0c */
#define MDIO_CNTRL_BUSY                  1
} EnetCoreMdio;

#define ENET_CORE0_UNIMAC ((volatile EnetCoreUnimac * const) ENET_CORE0_BASE)
#define ENET_CORE0_MIB ((volatile EnetCoreMib * const) (ENET_CORE0_BASE + 0x200))
#define ENET_CORE0_IF ((volatile EnetCoreIf * const) (ENET_CORE0_BASE + 0x400))
#define ENET_CORE0_MDIO ((volatile EnetCoreMdio * const) (ENET_CORE0_BASE + 0x600))

#define ENET_CORE1_UNIMAC ((volatile EnetCoreUnimac * const) ENET_CORE1_BASE)
#define ENET_CORE1_MIB ((volatile EnetCoreMib * const) (ENET_CORE1_BASE + 0x200))
#define ENET_CORE1_IF ((volatile EnetCoreIf * const) (ENET_CORE1_BASE + 0x400))




#define IUDMA_MAX_CHANNELS          2

/*
** DMA Channel Configuration (1 .. 32)
*/
typedef struct DmaChannelCfg {
    uint32        cfg;                      /* (00) assorted configuration */
#define         DMA_ENABLE      0x00000001  /* set to enable channel */
#define         DMA_PKT_HALT    0x00000002  /* idle after an EOP flag is detected */
#define         DMA_BURST_HALT  0x00000004  /* idle after finish current memory burst */
    uint32        intStat;                  /* (04) interrupts control and status */
    uint32        intMask;                  /* (08) interrupts mask */
#define         DMA_BUFF_DONE   0x00000001  /* buffer done */
#define         DMA_DONE        0x00000002  /* packet xfer complete */
#define         DMA_NO_DESC     0x00000004  /* no valid descriptors */
#define         DMA_RX_ERROR    0x00000008  /* rxdma detect client protocol error */
    uint32        maxBurst;                 /* (0C) max burst length permitted */
#define         DMA_DESCSIZE_SEL 0x00040000 /* DMA Descriptor Size Selection */
} DmaChannelCfg;

/*
** DMA State RAM (1 .. 16)
*/
typedef struct DmaStateRam {
    uint32        baseDescPtr;            /* (00) descriptor ring start address */
    uint32        state_data;             /* (04) state/bytes done/ring offset */
    uint32        desc_len_status;        /* (08) buffer descriptor status and len */
    uint32        desc_base_bufptr;       /* (0C) buffer descrpitor current processing */
} DmaStateRam;


/*
** DMA Registers
*/
typedef struct DmaRegs {
    uint32 controller_cfg;              /* (00) controller configuration */
#define DMA_MASTER_EN           0x00000001
#define DMA_FLOWC_CH1_EN        0x00000002
#define DMA_FLOWC_CH3_EN        0x00000004

    // Flow control Ch1
    uint32 flowctl_ch1_thresh_lo;           /* 004 */
    uint32 flowctl_ch1_thresh_hi;           /* 008 */
    uint32 flowctl_ch1_alloc;               /* 00c */
#define DMA_BUF_ALLOC_FORCE     0x80000000

    // Flow control Ch3
    uint32 flowctl_ch3_thresh_lo;           /* 010 */
    uint32 flowctl_ch3_thresh_hi;           /* 014 */
    uint32 flowctl_ch3_alloc;               /* 018 */

    // Flow control Ch5
    uint32 flowctl_ch5_thresh_lo;           /* 01C */
    uint32 flowctl_ch5_thresh_hi;           /* 020 */
    uint32 flowctl_ch5_alloc;               /* 024 */

    // Flow control Ch7
    uint32 flowctl_ch7_thresh_lo;           /* 028 */
    uint32 flowctl_ch7_thresh_hi;           /* 02C */
    uint32 flowctl_ch7_alloc;               /* 030 */

    uint32 ctrl_channel_reset;              /* 034 */
    uint32 ctrl_channel_debug;              /* 038 */
    uint32 reserved1;                       /* 03C */
    uint32 ctrl_global_interrupt_status;    /* 040 */
    uint32 ctrl_global_interrupt_mask;      /* 044 */

    // Unused words
    uint8 reserved2[0x200-0x48];

    // Per channel registers/state ram
    DmaChannelCfg chcfg[IUDMA_MAX_CHANNELS];/* Channel configuration */

    uint8 reserved3[0x400-0x220];

    union {
        DmaStateRam     s[IUDMA_MAX_CHANNELS];
        uint32          u32[4 * IUDMA_MAX_CHANNELS];
    } stram;                                /* (400-5FF) state ram */
} DmaRegs;

#define ETH0_DMA ((volatile DmaRegs * const) ETH0_DMA_BASE)
#define ETH1_DMA ((volatile DmaRegs * const) ETH1_DMA_BASE)
#define BRIDGE_DMA ((volatile DmaRegs * const) BRIDGE_DMA_BASE)


/*
** DMA Buffer
*/
typedef struct DmaDesc {
#if defined(__MIPSEL)
    uint32        address;                /* address of data */
    union {
        struct {
            uint16        status;                   /* buffer status */
#define          DMA_OWN                0x8000      /* cleared by DMA, set by SW */
#define          DMA_EOP                0x4000      /* last buffer in packet */
#define          DMA_SOP                0x2000      /* first buffer in packet */
#define          DMA_WRAP               0x1000
#define          DMA_PRIO               0x0C00      /* Unused in Duna */
#define          DMA_APPEND_BRCM_TAG    0x0200      /* Unused in Duna */
#define          DMA_DESC_TS            0x400
#define          DMA_DESC_ERROR         0x800
#define          DMA_APPEND_CRC         0x100       /* Unused in Duna */
            uint16        length;                   /* in bytes of data in buffer */
#define          DMA_DESC_USEFPM    0x8000
#define          DMA_DESC_BUFLENGTH 0x0fff
        };
        uint32      word0;
    };
#else
    union {
        struct {
            uint16        length;                   /* in bytes of data in buffer */
#define          DMA_DESC_USEFPM    0x8000
#define          DMA_DESC_BUFLENGTH 0x0fff
            uint16        status;                   /* buffer status */
#define          DMA_OWN                0x8000      /* cleared by DMA, set by SW */
#define          DMA_EOP                0x4000      /* last buffer in packet */
#define          DMA_SOP                0x2000      /* first buffer in packet */
#define          DMA_WRAP               0x1000
#define          DMA_PRIO               0x0C00      /* Unused in Duna */
#define          DMA_APPEND_BRCM_TAG    0x0200      /* Unused in Duna */
#define          DMA_DESC_TS            0x400
#define          DMA_DESC_ERROR         0x800
#define          DMA_APPEND_CRC         0x100       /* Unused in Duna */
        };
        uint32      word0;
    };
    uint32        address;                /* address of data */
#endif
} DmaDesc;

/*
** 16 Byte DMA Buffer
*/
typedef struct {
#if defined(__MIPSEL)
    /* BEWARE: This Descriptor format is still untested.
    ** The ordering of fields reserved, control and address is a guess.
    */
    uint32        reserved;
    uint32        control;
    uint32        address;                 /* address of data */
    union {
        struct {
            uint16        status;                   /* buffer status */
#define          DMA_OWN                0x8000      /* cleared by DMA, set by SW */
#define          DMA_EOP                0x4000      /* last buffer in packet */
#define          DMA_SOP                0x2000      /* first buffer in packet */
#define          DMA_WRAP               0x1000
#define          DMA_PRIO               0x0C00      /* Unused in Duna */
#define          DMA_APPEND_BRCM_TAG    0x0200      /* Unused in Duna */
#define          DMA_DESC_TS            0x400
#define          DMA_DESC_ERROR         0x800
#define          DMA_APPEND_CRC         0x100       /* Unused in Duna */
            uint16        length;                   /* in bytes of data in buffer */
#define          DMA_DESC_USEFPM        0x8000
#define          DMA_DESC_BUFLENGTH     0x0fff
        };
        uint32      word0;
    };
#define         GEM_ID_MASK             0x001F
#else
    union {
        struct {
            uint16        length;                   /* in bytes of data in buffer */
#define          DMA_DESC_USEFPM        0x8000
#define          DMA_DESC_BUFLENGTH     0x0fff
            uint16        status;                   /* buffer status */
#define          DMA_OWN                0x8000      /* cleared by DMA, set by SW */
#define          DMA_EOP                0x4000      /* last buffer in packet */
#define          DMA_SOP                0x2000      /* first buffer in packet */
#define          DMA_WRAP               0x1000
#define          DMA_PRIO               0x0C00      /* Unused in Duna */
#define          DMA_APPEND_BRCM_TAG    0x0200      /* Unused in Duna */
#define          DMA_DESC_TS            0x400
#define          DMA_DESC_ERROR         0x800
#define          DMA_APPEND_CRC         0x100       /* Unused in Duna */
        };
        uint32      word0;
    };

    uint32        address;                 /* address of data */
    uint32        control;
#define         GEM_ID_MASK             0x001F
    uint32        reserved;
#endif
} DmaDesc16;




/*
** PCI-E
*/
#define UBUS2_PCIE    /* define for UBUS2 architecture */

typedef struct PcieRegs{
    uint32 devVenID;
    uint16 command;
    uint16 status;
    uint32 revIdClassCode;
    uint32 headerTypeLatCacheLineSize;
    uint32 bar1;
    uint32 bar2;
    uint32 priSecBusNo;
#define PCIE_CFG_TYPE1_PRI_SEC_BUS_NO_SUB_BUS_NO_MASK              0x00ff0000
#define PCIE_CFG_TYPE1_PRI_SEC_BUS_NO_SUB_BUS_NO_SHIFT             16
#define PCIE_CFG_TYPE1_PRI_SEC_BUS_NO_SEC_BUS_NO_MASK              0x0000ff00
#define PCIE_CFG_TYPE1_PRI_SEC_BUS_NO_SEC_BUS_NO_SHIFT             8
#define PCIE_CFG_TYPE1_PRI_SEC_BUS_NO_PRI_BUS_NO_MASK              0x000000ff

    uint32 secStatusIoBaseLimit;
    uint32 rcMemBaseLimit;
    uint32 rcPrefBaseLimit;
    uint32 rcPrefBaseHi;
    uint32 rcPrefLimitHi;
    uint32 rcIoBaseLimit;
    uint32 capPointer;
    uint32 expRomBase;
    uint32 bridgeCtrl;
    uint32 unused1[27];

    /* PcieExpressCtrlRegs */
    uint16 pciExpressCap;
    uint16 pcieCapabilitiy;
    uint32 deviceCapability;
    uint16 deviceControl;
    uint16 deviceStatus;
    uint32 linkCapability;
    uint16 linkControl;
    uint16 linkStatus;
    uint32 slotCapability;
    uint16 slotControl;
    uint16 slotStatus;
    uint16 rootControl;
    uint16 rootCap;
    uint32 rootStatus;
    uint32 deviceCapability2;
    uint16 deviceControl2;
    uint16 deviceStatus2;
    uint32 linkCapability2;
    uint16 linkControl2;
    uint16 linkStatus2;
    uint32 slotCapability2;
    uint16 slotControl2;
    uint16 slotStatus2;
    uint32 unused2[6];

    /* PcieErrorRegs */
    uint16 advErrCapId;
    uint16 advErrCapOff;
    uint32 ucErrStatus;
    uint32 ucorrErrMask;
    uint32 ucorrErrSevr;
    uint32 corrErrStatus;
    uint32 corrErrMask;
    uint32 advErrCapControl;
    uint32 headerLog1;
    uint32 headerLog2;
    uint32 headerLog3;
    uint32 headerLog4;
    uint32 rootErrorCommand;
    uint32 rootErrorStatus;
    uint32 rcCorrId;
    uint32 rcFatalNonfatalId;
    uint32 unused3[10];

    /* PcieVcRegs */
    uint16 vcCapId;
    uint16 vcCapOffset;
    uint32 prtVcCapability;
    uint32 portVcCapability2;
    uint16 portVcControl;
    uint16 portVcCtatus;
    uint32 portArbStatus;
    uint32 vcRsrcControl;
    uint32 vcRsrcStatus;
} PcieRegs;

typedef struct PcieRcCfgVendorRegs{
    uint32 vendorCap;
    uint32 specificHeader;
    uint32 specificReg1;
#define PCIE_RC_CFG_VENDOR_SPECIFIC_REG1_ENDIAN_MODE_BAR1_SHIFT   0
#define PCIE_RC_CFG_VENDOR_SPECIFIC_REG1_ENDIAN_MODE_BAR2_SHIFT   2
#define PCIE_RC_CFG_VENDOR_SPECIFIC_REG1_ENDIAN_MODE_BAR3_SHIFT   4
#define PCIE_RC_CFG_VENDOR_SPECIFIC_REG1_ENDIAN_MODE_WORD_ALIGN   0x0
#define PCIE_RC_CFG_VENDOR_SPECIFIC_REG1_ENDIAN_MODE_HWORD_ALIGN  0x1
#define PCIE_RC_CFG_VENDOR_SPECIFIC_REG1_ENDIAN_MODE_BYTE_ALIGN   0x2
}PcieRcCfgVendorRegs;

typedef struct PcieBlk404Regs{
    uint32 unused;          /* 0x404 */
    uint32 config2;         /* 0x408 */
#define PCIE_IP_BLK404_CONFIG_2_BAR1_SIZE_MASK         0x0000000f
#define PCIE_IP_BLK404_CONFIG_2_BAR1_DISABLE           0
    uint32 config3;         /* 0x40c */
    uint32 pmDataA;         /* 0x410 */
    uint32 pmDataB;         /* 0x414 */
} PcieBlk404Regs;

typedef struct PcieBlk428Regs{
    uint32 vpdIntf;        /* 0x428 */
    uint16 unused_g;       /* 0x42c */
    uint16 vpdAddrFlag;    /* 0x42e */
    uint32 vpdData;        /* 0x430 */
    uint32 idVal1;         /* 0x434 */
    uint32 idVal2;         /* 0x438 */
    uint32 idVal3;         /* 0x43c */
#define PCIE_IP_BLK428_ID_VAL3_REVISION_ID_MASK                    0xff000000
#define PCIE_IP_BLK428_ID_VAL3_REVISION_ID_SHIFT                   24
#define PCIE_IP_BLK428_ID_VAL3_CLASS_CODE_MASK                     0x00ffffff
#define PCIE_IP_BLK428_ID_VAL3_CLASS_CODE_SHIFT                    16
#define PCIE_IP_BLK428_ID_VAL3_SUB_CLASS_CODE_SHIFT                 8

    uint32 idVal4;
    uint32 idVal5;
    uint32 unused_h;
    uint32 idVal6;
    uint32 msiData;
    uint32 msiAddr_h;
    uint32 msiAddr_l;
    uint32 msiMask;
    uint32 msiPend;
    uint32 pmData_c;
    uint32 msixControl;
    uint32 msixTblOffBir;
    uint32 msixPbaOffBit;
    uint32 unused_k;
    uint32 pcieCapability;
    uint32 deviceCapability;
    uint32 unused_l;
    uint32 linkCapability;
    uint32 bar2Config;
    uint32 pcieDeviceCapability2;
    uint32 pcieLinkCapability2;
    uint32 pcieLinkControl;
    uint32 pcieLinkCapabilityRc;
    uint32 bar3Config;
    uint32 rootCap;
    uint32 devSerNumCapId;
    uint32 lowerSerNum;
    uint32 upperSerNum;
    uint32 advErrCap;
    uint32 pwrBdgtData0;
    uint32 pwrBdgtData1;
    uint32 pwrBdgtData2;
    uint32 pwdBdgtData3;
    uint32 pwrBdgtData4;
    uint32 pwrBdgtData5;
    uint32 pwrBdgtData6;
    uint32 pwrBdgtData7;
    uint32 pwrBdgtCapability;
    uint32 vsecHdr;
    uint32 rcUserMemLo1;
    uint32 rcUserMemHi1;
    uint32 rcUserMemLo2;
    uint32 rcUserMemHi2;
} PcieBlk428Regs;

typedef struct PcieBlk800Regs{
#define NUM_PCIE_BLK_800_CTRL_REGS  6
    uint32 tlControl[NUM_PCIE_BLK_800_CTRL_REGS];
    uint32 tlCtlStat0;
    uint32 pmStatus0;
    uint32 pmStatus1;

#define NUM_PCIE_BLK_800_TAGS       32
    uint32 tlStatus[NUM_PCIE_BLK_800_TAGS];
    uint32 tlHdrFcStatus;
    uint32 tlDataFcStatus;
    uint32 tlHdrFcconStatus;
    uint32 tlDataFcconStatus;
    uint32 tlTargetCreditStatus;
    uint32 tlCreditAllocStatus;
    uint32 tlSmlogicStatus;
} PcieBlk800Regs;


typedef struct PcieBlk1000Regs{
#define NUM_PCIE_BLK_1000_PDL_CTRL_REGS  16
    uint32 pdlControl[NUM_PCIE_BLK_1000_PDL_CTRL_REGS];
    uint32 dlattnVec;
    uint32 dlAttnMask;
    uint32 dlStatus;        /* 0x1048 */
#define PCIE_IP_BLK1000_DL_STATUS_PHYLINKUP_MASK                   0x00002000
#define PCIE_IP_BLK1000_DL_STATUS_PHYLINKUP_SHIFT                  13
    uint32 dlTxChecksum;
    uint32 dlForcedUpdateGen1;
    uint32 reserved[43];
    uint32 mdioAddr;
    uint32 mdioWrData;
    uint32 mdioRdData;
    uint32 dlRxPFcCl;
    uint32 dlRxCFcCl;
    uint32 dlRxAckNack;
    uint32 dlTxRxSeqnb;
    uint32 dlTxPFcAl;
    uint32 dlTxNpFcAl;
    uint32 regDlSpare;
    uint32 dlRegSpare;
    uint32 dlTxRxSeq;
    uint32 dlRxNpFcCl;
} PcieBlk1000Regs;

typedef struct PcieBlk1800Regs{
#define NUM_PCIE_BLK_1800_PHY_CTRL_REGS         8
    uint32 phyCtrl[NUM_PCIE_BLK_1800_PHY_CTRL_REGS];
#define REG_POWERDOWN_P1PLL_ENA                      (1<<12)
} PcieBlk1800Regs;

typedef struct PcieRegs_EP{
    uint32 devVenID;
    uint16 command;
    uint16 status;
    uint32 revIdClassCode;
    uint32 headerTypeLatCacheLineSize;
    uint32 bar1;
    uint32 bar2;
    uint32 bar3;
    uint32 bar4;
    uint32 bar5;
    uint32 bar6;
    uint32 cardbusCis;
    uint32 subsystemVenID;
    uint32 expRomBar;
#define EXP_ROM_BAR_ENA_SHIFT     0
#define EXP_ROM_BAR_ENA_MASK      (1 << EXP_ROM_BAR_ENA_SHIFT)
#define EXP_ROM_BAR_LOW_SHIFT     1
#define EXP_ROM_BAR_LOW_MASK      (0x3ff << EXP_ROM_BAR_LOW_SHIFT)
#define EXP_ROM_BAR_SIZE_SHIFT    11
#define EXP_ROM_BAR_SIZE_MASK     (0x1fff << EXP_ROM_BAR_SIZE_SHIFT)
#define EXP_ROM_BAR_ADDR_SHIFT    24
#define EXP_ROM_BAR_ADDR_MASK     (0xff << EXP_ROM_BAR_ADDR_SHIFT)

    uint32 capPointer;
    uint32 reserved;
    uint32 latMinGrantIntPinIntLine;
#define LAT_INT_LINE_SHIFT      0
#define LAT_INT_LINE_MASK       (0xff << LAT_INT_LINE_SHIFT)
#define LAT_INT_PIN_SHIFT       8
#define LAT_INT_PIN_MASK        (0xff << LAT_INT_PIN_SHIFT)
#define LAT_MIN_GRANT_SHIFT     16
#define LAT_MIN_GRANT_MASK      (0xff << LAT_MIN_GRANT_SHIFT)
#define LAT_MAX_LATENCY_SHIFT   24
#define LAT_MAX_LATENCY_MASK    (0xff << LAT_MAX_LATENCY_SHIFT)

    uint32 unused1[27];

    /* PcieExpressCtrlRegs */
    uint16 pciExpressCap;
    uint16 pcieCapabilitiy;
    uint32 deviceCapability;
    uint16 deviceControl;
    uint16 deviceStatus;
    uint32 linkCapability;
    uint16 linkControl;
    uint16 linkStatus;
    uint32 slotCapability;
    uint16 slotControl;
    uint16 slotStatus;
    uint16 rootControl;
    uint16 rootCap;
    uint32 rootStatus;
    uint32 deviceCapability2;
    uint16 deviceControl2;
    uint16 deviceStatus2;
    uint32 linkCapability2;
    uint16 linkControl2;
    uint16 linkStatus2;
    uint32 slotCapability2;
    uint16 slotControl2;
    uint16 slotStatus2;
    uint32 unused2[6];

    /* PcieErrorRegs */
    uint16 advErrCapId;
    uint16 advErrCapOff;
    uint32 ucErrStatus;
    uint32 ucorrErrMask;
    uint32 ucorrErrSevr;
    uint32 corrErrStatus;
    uint32 corrErrMask;
    uint32 advErrCapControl;
    uint32 headerLog1;
    uint32 headerLog2;
    uint32 headerLog3;
    uint32 headerLog4;
    uint32 unused3[4];

    /* PcieDevRegs */
    uint32 devSerNumCap;
#define DEV_SER_NUM_CAP_ID_SHIFT    0
#define DEV_SER_NUM_CAP_ID_MASK     (0xffff << DEV_SER_NUM_CAP_ID_SHIFT)
#define DEV_VER_SHIFT               16
#define DEV_VER_MASK                (0xf << DEV_VER_SHIFT)
#define DEV_NEXT_SHIFT              20
#define DEV_NEXT_MASK               (0xfff << DEV_NEXT_SHIFT)
    uint32 devLowerSerNum;
    uint32 devUpperSerNum;
    uint32 unused4;

    /* PciePBRegs */
    uint32 pwrBdgtCap;
#define PWR_BDGT_CAP_ID_SHIFT       0
#define PWR_BDGT_CAP_ID_MASK        (0xffff << PWR_BDGT_CAP_ID_SHIFT)
#define PWR_BDGT_VER_SHIFT          16
#define PWR_BDGT_VER_MASK           (0xf << PWR_BDGT_VER_MASK)
#define PWR_BDGT_NEXT_SHIFT         20
#define PWR_BDGT_NEXT_MASK          (0xfff << PWR_BDGT_NEXT_MASK)

    uint32 pwrBdgtDataSel;
    uint32 pwrBdgtData;
#define PWR_BDGT_DATA_BASE_PWR_SHIFT  0
#define PWR_BDGT_DATA_BASE_PWR_MASK   (0xff << PWR_BDGT_DATA_BASE_PWR_SHIFT)
#define PWR_BDGT_DATA_DSCALE_SHIFT    8
#define PWR_BDGT_DATA_DSCALE_MASK     (0x3 << PWR_BDGT_DATA_DSCALE_SHIFT)
#define PWR_BDGT_DATA_PMSTATE_SHIFT   13
#define PWR_BDGT_DATA_PMSTATE_MASK    (0x3 << PWR_BDGT_DATA_PMSTATE_SHIFT)
#define PWR_BDGT_DATA_TYPE_SHIFT      15
#define PWR_BDGT_DATA_TYPE_MASK       (0x7 << PWR_BDGT_DATA_TYPE_SHIFT)
#define PWR_BDGT_DATA_RAIL_SHIFT      18
#define PWR_BDGT_DATA_RAIL_MASK       (0x7 << PWR_BDGT_DATA_RAIL_SHIFT)

    uint32 pwrBdgtCapability;

    /* PcieVcRegs */
    uint16 vcCapId;
    uint16 vcCapOffset;
    uint32 prtVcCapability;
    uint32 portVcCapability2;
    uint16 portVcControl;
    uint16 portVcCtatus;
    uint32 portArbStatus;
    uint32 vcRsrcControl;
    uint32 vcRsrcStatus;
} PcieRegs_EP;


typedef struct PcieMiscRegs{
    uint32 reset_ctrl;                    /* 4000 Reset Control Register */
    uint32 eco_ctrl_core;                 /* 4004 ECO Core Reset Control Register */
    uint32 misc_ctrl;                     /* 4008 MISC Control Register */
#define PCIE_MISC_CTRL_CFG_READ_UR_MODE                            (1<<13)
    uint32 cpu_2_pcie_mem_win0_lo;        /* 400c CPU to PCIe Memory Window 0 Low */
#define PCIE_MISC_CPU_2_PCI_MEM_WIN_LO_BASE_ADDR_MASK              0xfff00000
#define PCIE_MISC_CPU_2_PCI_MEM_WIN_LO_BASE_ADDR_SHIFT             20
#define PCIE_MISC_CPU_2_PCIE_MEM_ENDIAN_MODE_NO_SWAP               0
#define PCIE_MISC_CPU_2_PCIE_MEM_ENDIAN_MODE_HALF_WORD_SWAP        1
#define PCIE_MISC_CPU_2_PCIE_MEM_ENDIAN_MODE_HALF_BYTE_SWAP        2
    uint32 cpu_2_pcie_mem_win0_hi;        /* 4010 CPU to PCIe Memory Window 0 High */
    uint32 cpu_2_pcie_mem_win1_lo;        /* 4014 CPU to PCIe Memory Window 1 Low */
    uint32 cpu_2_pcie_mem_win1_hi;        /* 4018 CPU to PCIe Memory Window 1 High */
    uint32 cpu_2_pcie_mem_win2_lo;        /* 401c CPU to PCIe Memory Window 2 Low */
    uint32 cpu_2_pcie_mem_win2_hi;        /* 4020 CPU to PCIe Memory Window 2 High */
    uint32 cpu_2_pcie_mem_win3_lo;        /* 4024 CPU to PCIe Memory Window 3 Low */
    uint32 cpu_2_pcie_mem_win3_hi;        /* 4028 CPU to PCIe Memory Window 3 High */
    uint32 rc_bar1_config_lo;             /* 402c RC BAR1 Configuration Low Register */
#define  PCIE_MISC_RC_BAR_CONFIG_LO_MATCH_ADDRESS_MASK             0xfff00000
#define  PCIE_MISC_RC_BAR_CONFIG_LO_SIZE_256MB                     0xd
#define  PCIE_MISC_RC_BAR_CONFIG_LO_SIZE_MAX                       0x11     /* max is 4GB */
    uint32 rc_bar1_config_hi;             /* 4030 RC BAR1 Configuration High Register */
    uint32 rc_bar2_config_lo;             /* 4034 RC BAR2 Configuration Low Register */
    uint32 rc_bar2_config_hi;             /* 4038 RC BAR2 Configuration High Register */
    uint32 rc_bar3_config_lo;             /* 403c RC BAR3 Configuration Low Register */
    uint32 rc_bar3_config_hi;             /* 4040 RC BAR3 Configuration High Register */
    uint32 msi_bar_config_lo;             /* 4044 Message Signaled Interrupt Base Address Low Register */
    uint32 msi_bar_config_hi;             /* 4048 Message Signaled Interrupt Base Address High Register */
    uint32 msi_data_config;               /* 404c Message Signaled Interrupt Data Configuration Register */
    uint32 rc_bad_address_lo;             /* 4050 RC Bad Address Register Low */
    uint32 rc_bad_address_hi;             /* 4054 RC Bad Address Register High */
    uint32 rc_bad_data;                   /* 4058 RC Bad Data Register */
    uint32 rc_config_retry_timeout;       /* 405c RC Configuration Retry Timeout Register */
    uint32 eoi_ctrl;                      /* 4060 End of Interrupt Control Register */
    uint32 pcie_ctrl;                     /* 4064 PCIe Control */
    uint32 pcie_status;                   /* 4068 PCIe Status */
    uint32 revision;                      /* 406c PCIe Revision */
    uint32 cpu_2_pcie_mem_win0_base_limit;/* 4070 CPU to PCIe Memory Window 0 base/limit */
#define PCIE_MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_LIMIT_MASK        0xfff00000
#define PCIE_MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_LIMIT_SHIFT       20
#define PCIE_MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_BASE_MASK         0x0000fff0
#define PCIE_MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_BASE_SHIFT        4
    uint32 cpu_2_pcie_mem_win1_base_limit;/* 4074 CPU to PCIe Memory Window 1 base/limit */
    uint32 cpu_2_pcie_mem_win2_base_limit;/* 4078 CPU to PCIe Memory Window 2 base/limit */
    uint32 cpu_2_pcie_mem_win3_base_limit;/* 407c CPU to PCIe Memory Window 3 base/limit */
    uint32 ubus_ctrl;                     /* 4080 UBUS Control */
    uint32 ubus_timeout;                  /* 4084 UBUS Timeout */
    uint32 ubus_bar1_config_remap;        /* 4088 UBUS BAR1 System Bus Address Remap Register */
#define  PCIE_MISC_UBUS_BAR_CONFIG_OFFSET_MASK                      0xfff00000
#define  PCIE_MISC_UBUS_BAR_CONFIG_ACCESS_EN                        1
    uint32 ubus_bar2_config_remap;        /* 408c UBUS BAR2 System Bus Address Remap Register */
    uint32 ubus_bar3_config_remap;        /* 4090 UBUS BAR3 System Bus Address Remap Register */
    uint32 ubus_status;                   /* 4094 UBUS Status */
} PcieMiscRegs;

typedef struct PcieMiscPerstRegs{
    uint32 perst_eco_ctrl_perst;          /* 4100 ECO PCIE Reset Control Register */
    uint32 perst_eco_cce_status;          /* 4104 Config Copy Engine Status */
} PcieMiscPerstRegs;

typedef struct PcieMiscHardRegs{
    uint32 hard_eco_ctrl_hard;            /* 4200 ECO Hard Reset Control Register */
    uint32 hard_pcie_hard_debug;          /* 4204 PCIE Hard Debug Register */
#define PCIE_MISC_HARD_PCIE_HARD_DEBUG_SERDES_IDDQ                   (1<<23)
#define PCIE_MISC_HARD_PCIE_HARD_DEBUG_CLKREQ_DEBUG_ENABLE           (1<<1)
} PcieMiscHardRegs;

typedef struct PcieCpuIntr1Regs{
    uint32 status;
#define PCIE_CPU_INTR1_IPI_CPU_INTR                                  (1<<8)
#define PCIE_CPU_INTR1_PCIE_UBUS_CPU_INTR                            (1<<7)
#define PCIE_CPU_INTR1_PCIE_NMI_CPU_INTR                             (1<<6)
#define PCIE_CPU_INTR1_PCIE_INTR_CPU_INTR                            (1<<5)
#define PCIE_CPU_INTR1_PCIE_INTD_CPU_INTR                            (1<<4)
#define PCIE_CPU_INTR1_PCIE_INTC_CPU_INTR                            (1<<3)
#define PCIE_CPU_INTR1_PCIE_INTB_CPU_INTR                            (1<<2)
#define PCIE_CPU_INTR1_PCIE_INTA_CPU_INTR                            (1<<1)
#define PCIE_CPU_INTR1_PCIE_ERR_ATTN_CPU_INTR                        (1<<0)
    uint32 maskStatus;
    uint32 maskSet;
    uint32 maskClear;
} PcieCpuIntr1Regs;

typedef struct PcieExtCfgRegs{
    uint32 index;
#define PCIE_EXT_CFG_BUS_NUM_MASK                                     0x0ff00000
#define PCIE_EXT_CFG_BUS_NUM_SHIFT                                    20
#define PCIE_EXT_CFG_DEV_NUM_MASK                                     0x000f0000
#define PCIE_EXT_CFG_DEV_NUM_SHIFT                                    15
#define PCIE_EXT_CFG_FUNC_NUM_MASK                                    0x00007000
#define PCIE_EXT_CFG_FUNC_NUM_SHIFT                                   12
    uint32 data;
    uint32 scratch;
} PcieExtCfgRegs;

#define PCIEH                         ((volatile uint32 * const) PCIE_BASE)
#define PCIEH_REGS                    ((volatile PcieRegs * const) PCIE_BASE)
#define PCIEH_RC_CFG_VENDOR_REGS      ((volatile PcieRcCfgVendorRegs * const) \
                                        (PCIE_BASE+0x180)) 
#define PCIEH_BLK_404_REGS            ((volatile PcieBlk404Regs * const) \
                                        (PCIE_BASE+0x404))
#define PCIEH_BLK_428_REGS            ((volatile PcieBlk428Regs * const) \
                                        (PCIE_BASE+0x428))
#define PCIEH_BLK_800_REGS            ((volatile PcieBlk800Regs * const) \
                                        (PCIE_BASE+0x800))                                        
#define PCIEH_BLK_1000_REGS           ((volatile PcieBlk1000Regs * const) \
                                        (PCIE_BASE+0x1000))
#define PCIEH_BLK_1800_REGS           ((volatile PcieBlk1800Regs * const) \
                                        (PCIE_BASE+0x1800))
#define PCIEH_EP_REGS                 ((volatile PcieRegsEP * const) \
                                        (PCIE_BASE+0x2000))
#define PCIEH_EP_CFG_VENDOR_REGS      ((volatile PcieRcCfgVendorRegs * const) \
                                        (PCIE_BASE+0x2180))
#define PCIEH_EP_CFG_PRIV0            ((volatile PcieBlk404Regs * const) \
                                        (PCIE_BASE+0x2404))
#define PCIEH_EP_CFG_PRIV1            ((volatile PcieBlk428Regs * const) \
                                        (PCIE_BASE+0x2428))
#define PCIEH_EP_TL                   ((volatile PcieBlk800Regs * const) \
                                        (PCIE_BASE+0x2800))
#define PCIEH_EP_DL                   ((volatile PcieBlk1000Regs * const) \
                                        (PCIE_BASE+0x3000))
#define PCIEH_EP_BLK_1800_REGS        ((volatile PcieBlk1800Regs * const) \
                                        (PCIE_BASE+0x3800))
#define PCIEH_MISC_REGS               ((volatile PcieMiscRegs * const)  \
                                        (PCIE_BASE+0x4000))
#define PCIEH_MISC_PERST_REGS         ((volatile PcieMiscPerstRegs * const)  \
                                        (PCIE_BASE+0x4100))
#define PCIEH_MISC_HARD_REGS          ((volatile PcieMiscHardRegs * const)  \
                                        (PCIE_BASE+0x4200))
#define PCIEH_CPU_INTR1_REGS          ((volatile PcieCpuIntr1Regs * const)  \
                                        (PCIE_BASE+0x8300))
#define PCIEH_PCIE_EXT_CFG_REGS       ((volatile PcieExtCfgRegs * const)  \
                                        (PCIE_BASE+0x8400))
#define PCIEH_DEV_OFFSET              0x9000

#define PCIEH_RC_CFG_PRIV0            PCIEH_BLK_404_REGS
#define PCIEH_RC_CFG_PRIV1            PCIEH_BLK_428_REGS
#define PCIEH_RC_TL                   PCIEH_BLK_800_REGS
#define PCIEH_RC_DL                   PCIEH_BLK_1000_REGS
#define PCIEH_EXT_CFG_DATA            PCIEH_DEV_OFFSET

/*MEM1 size is 512MB, but no MEM2 in the chip, share half to MEM2 for code compatibility*/
#define PCIEH_MEM1_BASE               0xa0000000
#define PCIEH_MEM1_SIZE               0x10000000

#define PCIEH_MEM2_BASE               0xb0000000
#define PCIEH_MEM2_SIZE               0x10000000



/*
** Bridge Registers
*/

typedef struct {
    uint32 endianConversion;            /* 0x00 */
#define BRIDGE_ENDIAN_CONV_ENABLE       1
    uint32 regsPrio;                    /* 0x04 */
#define BRIDGE_REG_PRIO                 1
    uint32 regsWren;                    /* 0x08 */
#define BRIDGE_CHIP_UBUS_WREN           1
#define BRIDGE_PLCPHY_UBUS_WREN         0x2
    uint32 softReset;                   /* 0x0c */
#define BRIDGE_CU2PU_SRESET_N           1
#define BRIDGE_PU2CU_SRESET_N           0x2
    uint32 chipUbusMaxTrxSize;          /* 0x10 */
#define BRIDGE_CHIP_UBUS_MAX_TRX_SIZE   0xff
    uint32 plcphyUbusMaxTrxSize;        /* 0x14 */
#define BRIDGE_PLCPHY_UBUS_MAX_TRX_SIZE 0xff
} BridgeU2U;


typedef struct {
    uint32 channelCtrl;                 /* 0x00 */
#define BRIDGE_PLCPHY_ETH0_MBOX_EN_START_TX     1
#define BRIDGE_PLCPHY_ETH0_START_TX             (1 << 1)
#define BRIDGE_PLCPHY_ETH0_START_RX             (1 << 2)
#define BRIDGE_PLCPHY_ETH0_FREE_TX              (1 << 3)
#define BRIDGE_PLCPHY_ETH0_FREE_RX              (1 << 4)
#define BRIDGE_PLCPHY_ETH1_MBOX_EN_START_TX     (1 << 5)
#define BRIDGE_PLCPHY_ETH1_START_TX             (1 << 6)
#define BRIDGE_PLCPHY_ETH1_START_RX             (1 << 7)
#define BRIDGE_PLCPHY_ETH1_FREE_TX              (1 << 8)
#define BRIDGE_PLCPHY_ETH1_FREE_RX              (1 << 9)
#define BRIDGE_PLCPHY_SDR_MBOX_EN_START_TX      (1 << 10)
#define BRIDGE_PLCPHY_SDR_START_TX              (1 << 11)
#define BRIDGE_PLCPHY_SDR_START_RX              (1 << 12)
#define BRIDGE_PLCPHY_SDR_FREE_TX               (1 << 13)
#define BRIDGE_PLCPHY_SDR_FREE_RX               (1 << 14)

    uint32 channelStatus;               /* 0x04 */
#define BRIDGE_PLCPHY_ETH0_STATUS               1
#define BRIDGE_PLCPHY_ETH1_STATUS               (1 << 1)
#define BRIDGE_PLCPHY_SDR_STATUS                (1 << 2)

    uint32 channelSoftReset;            /* 0x08 */
#define BRIDGE_PLCPHY_ETH0_SRESET_N             1
#define BRIDGE_PLCPHY_ETH1_SRESET_N             (1 << 1)
#define BRIDGE_PLCPHY_SDR_SRESET_N              (1 << 2)

    uint32 txBufferLenEth0;             /* 0x0c */
#define BRIDGE_PLCPHY_TX_BUFFER_LEN             0xfff

    uint32 rxBufferLenEth0;             /* 0x10 */
#define BRIDGE_PLCPHY_RX_BUFFER_LEN             0xfff

    uint32 txBufferLenEth1;             /* 0x14 */
    uint32 rxBufferLenEth1;             /* 0x18 */
    uint32 txBufferLenSdr;              /* 0x1c */
    uint32 rxBufferLenSdr;              /* 0x20 */
    uint32 txBufferNumPrioIudmaEnEth0;  /* 0x24 */
#define BRIDGE_PLCPHY_TX_BUF_NUM_PRIO_IUDMA_EN  0x1f

    uint32 txBufferNumPrioIudmaEnEth1;  /* 0x28 */
    uint32 rxPktOffsetStart;            /* 0x2c */
#define BRIDGE_PLCPHY_RX_PKT_OFFSET_START       0x3f

    uint32 unused1[52];
    uint32 iudmaEthDesc0Low;            /* 0x100 */
    uint32 iudmaEthDesc0Up;             /* 0x104 */
#define BRIDGE_PLCPHY_IUDMADESC_PERIPH_STATUS   0xfff
#define BRIDGE_PLCPHY_IUDMADESC_W               (1 << 12)
#define BRIDGE_PLCPHY_IUDMADESC_S               (1 << 13)
#define BRIDGE_PLCPHY_IUDMADESC_E               (1 << 14)
#define BRIDGE_PLCPHY_IUDMADESC_O               (1 << 15)
#define BRIDGE_PLCPHY_IUDMADESC_BUF_LEN         (0xfff << 16)
#define BRIDGE_PLCPHY_IUDMADESC_SDR_STATUS      (1 << 31)

    uint32 iudmaEthDesc1Low;            /* 0x108 */
    uint32 iudmaEthDesc1Up;             /* 0x10c */
    uint32 iudmaEthDesc2Low;            /* 0x110 */
    uint32 iudmaEthDesc2Up;             /* 0x114 */
    uint32 iudmaEthDesc3Low;            /* 0x118 */
    uint32 iudmaEthDesc3Up;             /* 0x11c */
    uint32 iudmaEthDesc4Low;            /* 0x120 */
    uint32 iudmaEthDesc4Up;             /* 0x124 */
    uint32 iudmaEthDesc5Low;            /* 0x128 */
    uint32 iudmaEthDesc5Up;             /* 0x12c */
    uint32 iudmaEthDesc6Low;            /* 0x130 */
    uint32 iudmaEthDesc6Up;             /* 0x134 */
    uint32 iudmaEthDesc7Low;            /* 0x138 */
    uint32 iudmaEthDesc7Up;             /* 0x13c */
    uint32 iudmaEthDesc8Low;            /* 0x140 */
    uint32 iudmaEthDesc8Up;             /* 0x144 */
    uint32 iudmaEthDesc9Low;            /* 0x148 */
    uint32 iudmaEthDesc9Up;             /* 0x14c */
    uint32 iudmaEthDesc10Low;           /* 0x150 */
    uint32 iudmaEthDesc10Up;            /* 0x154 */
    uint32 iudmaEthDesc11Low;           /* 0x158 */
    uint32 iudmaEthDesc11Up;            /* 0x15c */
    uint32 iudmaEthDesc12Low;           /* 0x160 */
    uint32 iudmaEthDesc12Up;            /* 0x164 */
    uint32 iudmaEthDesc13Low;           /* 0x168 */
    uint32 iudmaEthDesc13Up;            /* 0x16c */
    uint32 iudmaEthDesc14Low;           /* 0x170 */
    uint32 iudmaEthDesc14Up;            /* 0x174 */
    uint32 iudmaEthDesc15Low;           /* 0x178 */
    uint32 iudmaEthDesc15Up;            /* 0x17c */
    uint32 iudmaEthDesc16Low;           /* 0x180 */
    uint32 iudmaEthDesc16Up;            /* 0x184 */
    uint32 iudmaEthDesc17Low;           /* 0x188 */
    uint32 iudmaEthDesc17Up;            /* 0x18c */
    uint32 iudmaEthDesc18Low;           /* 0x190 */
    uint32 iudmaEthDesc18Up;            /* 0x194 */
    uint32 iudmaEthDesc19Low;           /* 0x198 */
    uint32 iudmaEthDesc19Up;            /* 0x19c */
} BridgePLCPhyDatapaths;

typedef struct {
    uint32 ctrlEth0;                      /* 0x00 */
#define BRIDGE_CHDMA_ABORT                1
#define BRIDGE_CHDMA_SRESET_N             (1 << 1)
#define BRIDGE_CHDMA_VRT_CELL_SIZE        (0x3ff << 2)
#define BRIDGE_CHDMA_HDR_LEN              (0xff << 12)
#define BRIDGE_CHDMA_TX_BUFFER_TYPE       (0xf << 20)
#define BRIDGE_CHDMA_RX_BUFFER_TYPE       (0xf << 24)
#define BRIDGE_CHDMA_CELL_FLAT            (1 << 28)
#define BRIDGE_CHDMA_UBUS_PRIO_CTRL       (1 << 29)
#define BRIDGE_CHDMA_UBUS_PRIO_DATA       (1 << 30)
#define BRIDGE_CHDMA_UBUS_WR_ACK_EN_RX    (1 << 31)

    uint32 ctrlEth1;                      /* 0x04 */
    uint32 ctrlSdr;                       /* 0x08 */
    uint32 status;                        /* 0x0c */
#define BRIDGE_CHDMA_CHANNEL_BUSY         0x3

    uint32 irqCtrl;                       /* 0x10 */
#define BRIDGE_CHDMA_CHANNEL_IRQ_EN           0x3f
#define BRIDGE_CHDMA_CHANNEL_IRQ_EN_ETH0_TX   1
#define BRIDGE_CHDMA_CHANNEL_IRQ_EN_ETH0_RX   (1 << 1)
#define BRIDGE_CHDMA_CHANNEL_IRQ_EN_ETH1_TX   (1 << 2)
#define BRIDGE_CHDMA_CHANNEL_IRQ_EN_ETH1_RX   (1 << 3)
#define BRIDGE_CHDMA_CHANNEL_IRQ_EN_SDR_TX    (1 << 4)
#define BRIDGE_CHDMA_CHANNEL_IRQ_EN_SDR_RX    (1 << 5)
#define BRIDGE_CHDMA_CHANNEL_IRQ_CLR          (0x3f << 6)
#define BRIDGE_CHDMA_CHANNEL_IRQ_CLR_ETH0_TX  (1 << 6)
#define BRIDGE_CHDMA_CHANNEL_IRQ_CLR_ETH0_RX  (1 << 7)
#define BRIDGE_CHDMA_CHANNEL_IRQ_CLR_ETH1_TX  (1 << 8)
#define BRIDGE_CHDMA_CHANNEL_IRQ_CLR_ETH1_RX  (1 << 9)
#define BRIDGE_CHDMA_CHANNEL_IRQ_CLR_SDR_TX   (1 << 10)
#define BRIDGE_CHDMA_CHANNEL_IRQ_CLR_SDR_RX   (1 << 11)

    uint32 irqMaskEth0;                   /* 0x14 */
#define BRIDGE_CHDMA_TX_NE                1
#define BRIDGE_CHDMA_TX_PE                (1 << 1)
#define BRIDGE_CHDMA_TX_LK                (1 << 2)
#define BRIDGE_CHDMA_TX_AE                (1 << 3)
#define BRIDGE_CHDMA_TX_DL                (1 << 4)
#define BRIDGE_CHDMA_TX_RP                (1 << 5)
#define BRIDGE_CHDMA_TX_CE                (1 << 6)
#define BRIDGE_CHDMA_TX_OF                (1 << 7)
#define BRIDGE_CHDMA_TX_ME                (1 << 8)
#define BRIDGE_CHDMA_TX_BE                (1 << 9)
#define BRIDGE_CHDMA_TX_VE                (1 << 10)
#define BRIDGE_CHDMA_TX_OE                (1 << 11)
#define BRIDGE_CHDMA_TX_XE                (1 << 12)
#define BRIDGE_CHDMA_TX_LN                (1 << 13)
#define BRIDGE_CHDMA_TX_HE                (1 << 14)
#define BRIDGE_CHDMA_RX_PKT_ERR           (1 << 15)
#define BRIDGE_CHDMA_RX_NE                (1 << 16)
#define BRIDGE_CHDMA_RX_PE                (1 << 17)
#define BRIDGE_CHDMA_RX_LK                (1 << 18)
#define BRIDGE_CHDMA_RX_AE                (1 << 19)
#define BRIDGE_CHDMA_RX_DL                (1 << 20)
#define BRIDGE_CHDMA_RX_RP                (1 << 21)
#define BRIDGE_CHDMA_RX_CE                (1 << 22)
#define BRIDGE_CHDMA_RX_OF                (1 << 23)
#define BRIDGE_CHDMA_RX_ME                (1 << 24)
#define BRIDGE_CHDMA_RX_BE                (1 << 25)
#define BRIDGE_CHDMA_RX_VE                (1 << 26)
#define BRIDGE_CHDMA_RX_OE                (1 << 27)
#define BRIDGE_CHDMA_RX_XE                (1 << 28)
#define BRIDGE_CHDMA_RX_LN                (1 << 29)
#define BRIDGE_CHDMA_RX_HE                (1 << 30)
#define BRIDGE_CHDMA_RX_CELL_MAX          (1 << 31)

    uint32 irqMaskEth1;                   /* 0x18 */
    uint32 irqMaskSdr;                    /* 0x1c */
    uint32 irqStatEth0;                   /* 0x20 */
    uint32 irqStatEth1;                   /* 0x24 */
    uint32 irqStatSdr;                    /* 0x28 */
    uint32 burstSizeEth0;                 /* 0x2c */
#define BRIDGE_CHDMA_UBUS_BURST_SIZE      0x7f

    uint32 burstSizeEth1;                 /* 0x30 */
    uint32 burstSizeSdr;                  /* 0x34 */
    uint32 txIudmaBaseDescPtrAddrEth0;    /* 0x38 */
    uint32 txIudmaBaseDescPtrAddrEth1;    /* 0x3c */
    uint32 txIudmaBaseDescPtrAddrSdr;     /* 0x40 */
    uint32 rxIudmaBaseDescPtrAddrEth0;    /* 0x44 */
    uint32 rxIudmaBaseDescPtrAddrEth1;    /* 0x48 */
    uint32 rxIudmaBaseDescPtrAddrSdr;     /* 0x4c */
    uint32 mboxAddrEth0;                  /* 0x50 */
    uint32 mboxAddrEth1;                  /* 0x54 */
    uint32 mboxAddrSdr;                   /* 0x58 */
    uint32 txIudmaCtrlAddrEth0;           /* 0x5c */
    uint32 txIudmaCtrlAddrEth1;           /* 0x60 */
    uint32 txIudmaCtrlConfEth0;           /* 0x64 */
    uint32 txIudmaCtrlConfEth1;           /* 0x68 */
    uint32 txFlatBaseDescPtrAddrEth0;     /* 0x6c */
    uint32 rxFlatBaseDescPtrAddrEth0;     /* 0x70 */
    uint32 txFlatBaseDescPtrAddrEth1;     /* 0x74 */
    uint32 rxFlatBaseDescPtrAddrEth1;     /* 0x78 */
    uint32 txFlatBaseDescPtrAddrSdr;      /* 0x7c */
    uint32 rxFlatBaseDescPtrAddrSdr;      /* 0x80 */
    uint32 cellStatusResetEth0;           /* 0x84 */
#define BRIDGE_CHDMA_RST_TX_CELL_CNTR     1
#define BRIDGE_CHDMA_RST_RX_CELL_CNTR     (1 << 1)

    uint32 cellStatusResetEth1;           /* 0x88 */
    uint32 cellStatusResetSdr;            /* 0x8c */
    uint32 txCellStatusEth0;              /* 0x90 */
    uint32 txCellStatusEth1;              /* 0x94 */
    uint32 txCellStatusSdr;               /* 0x98 */
    uint32 rxCellStatusEth0;              /* 0x9c */
    uint32 rxCellStatusEth1;              /* 0xa0 */
    uint32 rxCellStatusSdr;               /* 0xa4 */
    uint32 chdmaStatus0Eth0;              /* 0xa8 */
    uint32 unused1;
    uint32 chdmaStatus1Eth0;              /* 0xb0 */
    uint32 chdmaStatus2Eth0;              /* 0xb4 */
    uint32 unused2;
    uint32 chdmaStatus3Eth0;              /* 0xbc */
    uint32 chdmaStatus0Eth1;              /* 0xc0 */
    uint32 chdmaStatus1Eth1;              /* 0xc4 */
    uint32 chdmaStatus2Eth1;              /* 0xc8 */
    uint32 chdmaStatus3Eth1;              /* 0xcc */
    uint32 chdmaStatus0Sdr;               /* 0xd0 */
    uint32 chdmaStatus1Sdr;               /* 0xd4 */
    uint32 chdmaStatus2Sdr;               /* 0xd8 */
    uint32 chdmaStatus3Sdr;               /* 0xdc */
} BridgeCHDMA;

typedef struct {
    uint32 ctrlConfig;                    /* 0x00 */
#define BRIDGE_CHDMA_IUDMA_EN             1
#define BRIDGE_CHDMA_FLOWC_CH1            (1 << 1)
#define BRIDGE_CHDMA_FLOWC_CH3            (1 << 2)
#define BRIDGE_CHDMA_FLOWC_CH5            (1 << 3)
#define BRIDGE_CHDMA_FLOWC_CH7            (1 << 4)
#define BRIDGE_CHDMA_RXDMA_PROT_EN        (1 << 5)

    uint32 ch1FlowCtrlLowThres;           /* 0x04 */
#define BRIDGE_CHDMA_FCLOWTHR             0xffff

    uint32 ch1FlowCtrlHighThres;          /* 0x08 */
#define BRIDGE_CHDMA_FCHIGHTHR            0xffff

    uint32 ch1FlowCtrlBufAlloc;           /* 0x0c */
#define BRIDGE_CHDMA_FCBUFALLOC           0xffff
#define BRIDGE_CHDMA_FORCE                (1 << 31)

    uint32 ch3FlowCtrlLowThres;           /* 0x10 */
    uint32 ch3FlowCtrlHighThres;          /* 0x14 */
    uint32 ch3FlowCtrlBufAlloc;           /* 0x18 */
    uint32 ch5FlowCtrlLowThres;           /* 0x1c */
    uint32 ch5FlowCtrlHighThres;          /* 0x20 */
    uint32 ch5FlowCtrlBufAlloc;           /* 0x24 */
    uint32 ch7FlowCtrlLowThres;           /* 0x28 */
    uint32 ch7FlowCtrlHighThres;          /* 0x2c */
    uint32 ch7FlowCtrlBufAlloc;           /* 0x30 */
    uint32 channelReset;                  /* 0x34 */
    uint32 channelDebug;                  /* 0x38 */
#define BRIDGE_CHDMA_DIAG_SEL_A           0x1f
#define BRIDGE_CHDMA_DIAG_SEL_B           (0x1f << 8)
    uint32 unused;                        /* 0x3c */
    uint32 glblIntStatus;                 /* 0x40 */
    uint32 glblIntMask;                   /* 0x44 */
} BridgeIUDMACtrl;

typedef struct {
    uint32 chConfig;                      /* 0x00 */
#define BRIDGE_CHDMA_EN_DMA               1
#define BRIDGE_CHDMA_PKT_HALT             (1 << 1)
#define BRIDGE_CHDMA_BURST_HALT           (1 << 2)

    uint32 chIntStatus;                   /* 0x04 */
#define BRIDGE_CHDMA_BUF_DONE             1
#define BRIDGE_CHDMA_PKT_DONE             (1 << 1)
#define BRIDGE_CHDMA_NOT_VALID            (1 << 2)
#define BRIDGE_CHDMA_RXDMA_ERROR          (1 << 3)
#define BRIDGE_CHDMA_REPIN_ERROR          (1 << 4)

    uint32 chIntMask;                     /* 0x08 */
    uint32 chIntMaxburstCfg;              /* 0x0c */
#define BRIDGE_CHDMA_MAX_BURST            0x3f
#define BRIDGE_CHDMA_FORCED_STARTOFF      (0x3 << 8)
#define BRIDGE_CHDMA_DISABLE_BUF_WWA      (1 << 16)
#define BRIDGE_CHDMA_AUTOKICK_EN          (1 << 17)
#define BRIDGE_CHDMA_DESCSIZE_SEL         (1 << 18)
#define BRIDGE_CHDMA_THROUGHPUT_TEST_EN   (1 << 19)
} BridgeIUDMAChannelCtrl;

typedef struct {
    uint32 chStateStram1;                 /* 0x00 */
    uint32 chStateStram2;                 /* 0x04 */
#define BRIDGE_CHDMA_RING_OFFSET          0x1fff
#define BRIDGE_CHDMA_BYTES_DONE           (0xfff << 16)
#define BRIDGE_CHDMA_STATE                (0x3 << 30)
    uint32 chStateStram3;                 /* 0x08 */
    uint32 chStateStram4;                 /* 0x0c */
} BridgeIUDMAChannelStateRam;

#define BRIDGE_U2U ((volatile BridgeU2U * const) BRIDGE_BASE)
#define BRIDGE_PLCPHY_DATAPATHS ((volatile BridgePLCPhyDatapaths * const) (BRIDGE_BASE + 0x10000))
#define BRIDGE_CHDMA ((volatile BridgeCHDMA * const) (BRIDGE_BASE + 0x11000))
#define BRIDGE_IUDMA_CTRL ((volatile BridgeIUDMACtrl * const) (BRIDGE_BASE + 0x20000))
#define BRIDGE_IUDMA_CH1 ((volatile BridgeIUDMAChannelCtrl * const) (BRIDGE_BASE + 0x20200))
#define BRIDGE_IUDMA_CH2 ((volatile BridgeIUDMAChannelCtrl * const) (BRIDGE_BASE + 0x20210))
#define BRIDGE_IUDMA_CH1_STATE ((volatile BridgeIUDMAChannelStateRam * const) (BRIDGE_BASE + 0x20400))
#define BRIDGE_IUDMA_CH2_STATE ((volatile BridgeIUDMAChannelStateRam * const) (BRIDGE_BASE + 0x20410))


/*
** Peripheral Registers
*/


/*
** Interrupt Controller
*/

#define IRQ_BITS 32
typedef struct  {
    uint32         IrqMask;
    uint32         IrqStatus;
    uint32         ExtIrqMask;
    uint32         ExtIrqStatus;
    uint32         Ext2IrqMask;
    uint32         Ext2IrqStatus;
    uint32         unused1[2];
    uint32         IrqMask1;
    uint32         IrqStatus1;
    uint32         ExtIrqMask1;
    uint32         ExtIrqStatus1;
    uint32         Ext2IrqMask1;
    uint32         Ext2IrqStatus1;
    uint32         unused2[2];
    uint32         IrqOutMask;
} IrqControl_t;

typedef struct PerfControl {
     uint32        ChipID;             /* (00) word 0 */
#define CHIP_VERSION_SHIFT      28
#define CHIP_VERSION_MASK       (0xf << CHIP_VERSION_SHIFT)
#define CHIP_PART_NUMBER_SHIFT  12
#define CHIP_PART_NUMBER_MASK   (0xffff << CHIP_PART_NUMBER_SHIFT)
#define CHIP_MANUF_ID_SHIFT     1
#define CHIP_MANUF_ID_MASK      (0x7ff << CHIP_MANUF_ID_SHIFT)

     uint32        blkEnables;        /* (04) word 1 */
#define SDR_CLK_EN              (1 << 6)
#define MIPS_UBUS_CLK_EN        (1 << 5)
#define MIPS_CLK_EN             (1 << 4)
#define PCIE_CLK_EN             (1 << 3)
#define GPHY_ENET_CLK_EN        (1 << 2)
#define ENET_0_CLK_EN           (1 << 1)
#define HS_SPI_CLK_EN           (1 << 0)

     uint32        timerControlReg;    /* (08) word 2 */
#define SOFT_RST    (1 << 0)

     uint32        deviceTimeoutEn;   /* (0c) word 3 */
     uint32        softResetB;        /* (10) word 4 */
#define SOFT_RST_MIPS           (1 << 8)
#define SOFT_RST_PLCPHY_BRIDGE  (1 << 7)
#define SOFT_RST_PCIE           (1 << 6)
#define SOFT_RST_PCIE_CORE      (1 << 5)
#define SOFT_RST_PCIE_EXT       (1 << 4)
#define SOFT_RST_GPHY_ENET      (1 << 3)
#define SOFT_RST_ENET_CORE0     (1 << 2)
#define SOFT_RST_HS_SPIM        (1 << 1)
#define SOFT_RST_HS_SPIM_PLL    (1 << 0)

    uint32        ExtIrqCfg;          /* (14) word 5*/
#define EI_INSENS_SHFT   0
#define EI_LEVEL_SHFT  4
#define EI_SENSE_SHFT   8
#define EI_STATUS_SHFT    12
#define EI_CLEAR_SHFT  16
#define EI_MASK_SHFT   20

    uint32        unused1;            /* (18) word 6 */
    uint32        unused2;            /* (1c) word 7 */
    uint32        unused3;            /* (20) word 7 */
    uint32        unused4;            /* (24) word 7 */
    uint32        unused5;            /* (28) word 7 */
    uint32        unused6;            /* (2c) word 7 */
    uint32        unused7;            /* (30) word 7 */
    uint32        unused8;            /* (34) word 7 */
    uint32        unused9;            /* (38) word 7 */
    uint32        unused10;           /* (3c) word 7 */

    union {
         IrqControl_t  IrqControl[1];  /* (40) */
    };
} PerfControl;

#define PERF ((volatile PerfControl * const) PERF_BASE)


/*
** Timer
*/
typedef struct Timer {
    uint32        TimerInts;
#define TIMER0          0x01
#define TIMER1          0x02
#define TIMER2          0x04
#define WATCHDOG        0x08
#define TIMER0_MASK     0x100
#define TIMER1_MASK     0x200
#define TIMER2_MASK     0x400

    uint32        TimerCtl0;
    uint32        TimerCtl1;
    uint32        TimerCtl2;
#define TIMERENABLE     0x80000000
#define RSTCNTCLR       0x40000000
    uint32        TimerCnt0;
    uint32        TimerCnt1;
    uint32        TimerCnt2;
#define TIMER_COUNT_MASK    0x3FFFFFFF
    uint32        WatchDogDefCount;

    /* Write 0xff00 0x00ff to Start timer
     * Write 0xee00 0x00ee to Stop and re-load default count
     * Read from this register returns current watch dog count
     */
    uint32        WatchDogCtl;

    /* Number of 50-MHz ticks for WD Reset pulse to last */
    uint32        WDResetCount;

    uint32        SoftRst;
#define SOFT_RESET              0x00000001      // 0
} Timer;

#define TIMER ((volatile Timer * const) TIMR_BASE)


/*
** Gpio Controller
*/

typedef struct GpioControl {
    uint32      GPIODir;                    /* 0 */
    uint32      GPIOData;                   /* 4 */
    uint32      GPIOFuncMode;               /* 8 */
    uint32      unused;
    uint32      GPIOMuxCtrl_0;              /* 10 */
#define GPIO_0_MUX_NSEL       0x7f
#define GPIO_1_MUX_NSEL       (0x7f << 8)
#define GPIO_2_MUX_NSEL       (0x7f << 16)
#define GPIO_3_MUX_NSEL       (0x7f << 24)

    uint32      GPIOMuxCtrl_1;              /* 14 */
#define GPIO_4_MUX_NSEL       0x7f
#define GPIO_5_MUX_NSEL       (0x7f << 8)
#define GPIO_6_MUX_NSEL       (0x7f << 16)
#define GPIO_7_MUX_NSEL       (0x7f << 24)

    uint32      GPIOMuxCtrl_2;              /* 18 */
#define GPIO_8_MUX_NSEL        0x7f
#define GPIO_9_MUX_NSEL        (0x7f << 8)
#define GPIO_10_MUX_NSEL       (0x7f << 16)
#define GPIO_11_MUX_NSEL       (0x7f << 24)

    uint32      GPIOMuxCtrl_3;              /* 1c */
#define GPIO_12_MUX_NSEL       0x7f
#define GPIO_13_MUX_NSEL       (0x7f << 8)
#define GPIO_14_MUX_NSEL       (0x7f << 16)
#define GPIO_15_MUX_NSEL       (0x7f << 24)

    uint32      GPIOMuxCtrl_4;              /* 20 */
#define GPIO_16_MUX_NSEL       0x7f
#define GPIO_17_MUX_NSEL       (0x7f << 8)
#define GPIO_18_MUX_NSEL       (0x7f << 16)
#define GPIO_19_MUX_NSEL       (0x7f << 24)

    uint32      GPIOMuxCtrl_5;              /* 24 */
#define GPIO_20_MUX_NSEL       0x7f
#define GPIO_21_MUX_NSEL       (0x7f << 8)
#define GPIO_22_MUX_NSEL       (0x7f << 16)
#define GPIO_23_MUX_NSEL       (0x7f << 24)

    uint32      GPIOMuxCtrl_6;              /* 28 */
#define GPIO_24_MUX_NSEL       0x7f
#define GPIO_25_MUX_NSEL       (0x7f << 8)
#define GPIO_26_MUX_NSEL       (0x7f << 16)
#define GPIO_27_MUX_NSEL       (0x7f << 24)

    uint32      GPIOMuxCtrl_7;              /* 2c */
#define GPIO_28_MUX_NSEL       0x7f
#define GPIO_29_MUX_NSEL       (0x7f << 8)
#define GPIO_30_MUX_NSEL       (0x7f << 16)
#define GPIO_31_MUX_NSEL       (0x7f << 24)
} GpioControl;

#define GPIO ((volatile GpioControl * const) GPIO_BASE)

#define GPIO_IN          0
#define GPIO_OUT         1
#define GPIO_DIRECT_MODE 0
#define GPIO_MUXED_MODE  1

/*
** Misc Register Set Definitions.
*/

typedef struct Misc {
    uint32 miscPeriphMbox;                      /* 0x00 */
#define MISC_PERIPH_MBOX_READY                  1
#define MISC_PERIPH_MBOX_MIPS_RESET             (1 << 1)
#define MISC_PERIPH_MBOX_IRQ                    (1 << 2)
    uint32 unused1[2];
    /* MISC STRAP Registers belong here, but they are defined
    ** separately to match the 6318 register map layout. Strap
    ** registers should only be accessed though the StrapControl
    ** struct below
    */
    uint32 miscTestControl;                     /* 0x0c */
#define TEST_CTRL_TEST_MODE                     0xf
#define TEST_CTRL_TEST_EN                       (1 << 11)
#define TEST_CTRL_TEST_IDDQ_EN                  (1 << 12)
#define TEST_CTRL_TEST_BLK_CTRL_EN              (1 << 13)
#define TEST_CTRL_TEST_BLK_CTRL                 (0x7fff << 14)

    uint32 miscTestPortBlkOEn0;                 /* 0x10 */
    uint32 miscTestPortBlkOEn1;                 /* 0x14 */
    uint32 miscTestPortBlkData0;                /* 0x18 */
    uint32 miscTestPortBlkData1;                /* 0x1c */
#define TP_BLK_DATA                             0xff

    uint32 miscTestPortCommand;                 /* 0x20 */
#define TP_COMMAND                              0xf
#define TP_RESET                                (1 << 4)

    uint32 miscTestDiagReadBackLo;              /* 0x24 */
    uint32 miscTestDiagReadBackHi;              /* 0x28 */
#define DIAG_READ_BACK_HI                       0xff
    uint32 miscPSMIddqCtrl;                     /* 0x2c */
#define IDDQ_CTRL_MIPS                          0x1
#define IDDQ_CTRL_PCIE                          0x2
#define IDDQ_CTRL_GPHY                          0x4

    uint32 miscPSMUbusActivePortsCtrl;          /* 0x30 */
#define UBUS_ACTIVE_SDRAM                       (1 << 0)
#define UBUS_ACTIVE_MIPS                        (1 << 1)
#define UBUS_ACTIVE_ENET0                       (1 << 2)
#define UBUS_ACTIVE_ENET1                       (1 << 3)
#define UBUS_ACTIVE_PCIE                        (1 << 5)
#define UBUS_ACTIVE_PLC_BRIDGE                  (1 << 8)
#define UBUS_ACTIVE_PLC_MASTER                  (1 << 9)
#define UBUS_ACTIVE_PLC_SLAVE                   (1 << 10)
#define UBUS_ACTIVE_UBUS_ERROR                  (1 << 14)
#define UBUS_ACTIVE_PERIPHS                     (1 << 15)
#define UBUS_ACTIVE_UNMAPPED_ID                 (1 << 31)

    uint32 miscIRQCtrl;                         /* 0x34 */
} Misc;

#define MISC ((volatile Misc * const) MISC_BASE)


/*
** Strap Override Control Registers
*/

typedef struct StrapControl {
     uint32  strapOverrideBus;                       /* 0x00 */
#define STRAP_BUS_PCIE_ONOFF_SHIFT                   16
#define STRAP_BUS_PCIE_ONOFF_MASK                    (1 << STRAP_BUS_PCIE_ONOFF_SHIFT)
#define STRAP_BUS_PCIE_RC_SHIFT                      15
#define STRAP_BUS_PCIE_RC_MASK                       (1 << STRAP_BUS_PCIE_RC_SHIFT)
#define STRAP_BUS_FP_BOOT_SHIFT                      13
#define STRAP_BUS_FP_BOOT_MASK                       (0x3 << STRAP_BUS_FP_BOOT_SHIFT)
#define STRAP_BUS_SDR_SHIFT                          12
#define STRAP_BUS_SDR_MASK                           (1 << STRAP_BUS_SDR_SHIFT)
#define STRAP_BUS_UBUS_FREQ_SHIFT                    11
#define STRAP_BUS_UBUS_FREQ_MASK                     (1 << STRAP_BUS_UBUS_FREQ_SHIFT)
#define STRAP_BUS_SDR_FREQ_SHIFT                     9
#define STRAP_BUS_SDR_FREQ_MASK                      (0x3 << STRAP_BUS_SDR_FREQ_SHIFT)
#define STRAP_BUS_MIPS_FREQ_SHIFT                    7
#define STRAP_BUS_MIPS_FREQ_MASK                     (0x3 << STRAP_BUS_MIPS_FREQ_SHIFT)
#define STRAP_BUS_ENET0_MODE_SHIFT                   6
#define STRAP_BUS_ENET0_MODE_MASK                    (1 << STRAP_BUS_ENET0_MODE_SHIFT)
#define STRAP_BUS_ENET0_ONOFF_SHIFT                  4
#define STRAP_BUS_ENET0_ONOFF_MASK                   (0x3 << STRAP_BUS_ENET0_ONOFF_SHIFT)
#define STRAP_BUS_GPHY_ONOFF_SHIFT                   3
#define STRAP_BUS_GPHY_ONOFF_MASK                    (1 << STRAP_BUS_GPHY_ONOFF_SHIFT)
#define STRAP_BUS_HSSPIM_FLASH_FASTREAD_SHIFT        2
#define STRAP_BUS_HSSPIM_FLASH_FASTREAD_MASK         (1 << STRAP_BUS_HSSPIM_FLASH_FASTREAD_SHIFT)
#define STRAP_BUS_HSSPIM_24_32_SHIFT                 1
#define STRAP_BUS_HSSPIM_24_32_MASK                  (1 << STRAP_BUS_HSSPIM_24_32_SHIFT)
#define STRAP_BUS_MIPS_BOOT_SHIFT                    0
#define STRAP_BUS_MIPS_BOOT_MASK                     1

     uint32  strapOverrideControl;                   /* 0x04 */
} StrapControl;

#define STRAP ((volatile StrapControl * const) STRAP_BASE)


/*
** BSTI Control Registers
*/

typedef struct BSTIControl {
    uint32 ser_ctrl;
#define BSTI_SER_CTRL_START_SHIFT                    28
#define BSTI_SER_CTRL_START_MASK                     (1 << BSTI_SER_CTRL_START_SHIFT)
#define BSTI_SER_CTRL_CMD_SHIFT                      26
#define BSTI_SER_CTRL_CMD_MASK                       (0x3 << BSTI_SER_CTRL_CMD_SHIFT)
#define BSTI_SER_CTRL_ADDR_SHIFT                     16
#define BSTI_SER_CTRL_ADDR_MASK                      (0x3ff << BSTI_SER_CTRL_ADDR_SHIFT)
#define BSTI_SER_CTRL_WR_DATA_SHIFT                  0
#define BSTI_SER_CTRL_WR_DATA_MASK                   (0xffff)

    uint32 ser_status;
#define BSTI_SER_STATUS_RD_DATA_MASK                 (0xffff)
} BSTIControl;

#define BSTI ((volatile BSTIControl * const) BSTI_BASE)

#define BSTI_START_OP                  0x01
#define BSTI_READ_OP                   0x02
#define BSTI_WRITE_OP                  0x01

#define AON_REGISTERS_LED_EN           0x340
#define AON_REGISTERS_LED0_ON          0x342
#define AON_REGISTERS_LED1_ON          0x344
#define AON_REGISTERS_LED0_OFF         0x346
#define AON_REGISTERS_LED1_OFF         0x348

/*
** UART
*/
typedef struct UartChannel {
#if defined(__MIPSEL)
    byte          fifoctl;
#define RSTTXFIFOS      0x80
#define RSTRXFIFOS      0x40
    /* 5-bit TimeoutCnt is in low bits of this register.
     *  This count represents the number of characters
     *  idle times before setting receive Irq when below threshold
     */
    byte          config;
#define XMITBREAK       0x40
#define BITS5SYM        0x00
#define BITS6SYM        0x10
#define BITS7SYM        0x20
#define BITS8SYM        0x30
#define ONESTOP         0x07
#define TWOSTOP         0x0f
    /* 4-LSBS represent STOP bits/char
     * in 1/8 bit-time intervals.  Zero
     * represents 1/8 stop bit interval.
     * Fifteen represents 2 stop bits.
     */
    byte          control;
#define BRGEN           0x80    /* Control register bit defs */
#define TXEN            0x40
#define RXEN            0x20
#define LOOPBK          0x10
#define TXPARITYEN      0x08
#define TXPARITYEVEN    0x04
#define RXPARITYEN      0x02
#define RXPARITYEVEN    0x01

    byte          unused0;
    uint32        baudword;
    /* When divide SysClk/2/(1+baudword) we should get 32*bit-rate
     */

    byte          prog_out;       /* Set value of DTR (Bit0), RTS (Bit1)
                                   *  if these bits are also enabled to GPIO_o
                                   */
#define DTREN   0x01
#define RTSEN   0x02
    byte          fifocfg;        /* Upper 4-bits are TxThresh, Lower are
                                   *      RxThreshold.  Irq can be asserted
                                   *      when rx fifo> thresh, txfifo<thresh
                                   */
    byte          rxf_levl;       /* Read-only fifo depth */
    byte          txf_levl;       /* Read-only fifo depth */

    byte          DeltaIP_SyncIP;         /* Upper 4 bits show which bits
                                           *  have changed (may set IRQ).
                                           *  read automatically clears bit
                                           * Lower 4 bits are actual status
                                           */
    byte          DeltaIPConfig_Mask;     /* Upper 4 bits: 1 for posedge sense
                                           *      0 for negedge sense if
                                           *      not configured for edge
                                           *      insensitive (see above)
                                           * Lower 4 bits: Mask to enable change
                                           *  detection IRQ for corresponding
                                           *  GPIO_i
                                           */
    byte          DeltaIPEdgeNoSense;     /* Low 4-bits, set corr bit to 1 to
                                           * detect irq on rising AND falling
                                           * edges for corresponding GPIO_i
                                           * if enabled (edge insensitive)
                                           */
    byte          unused1;

    uint16        intStatus;
#define DELTAIP         0x0001
#define TXUNDERR        0x0002
#define TXOVFERR        0x0004
#define TXFIFOTHOLD     0x0008
#define TXREADLATCH     0x0010
#define TXFIFOEMT       0x0020
#define RXUNDERR        0x0040
#define RXOVFERR        0x0080
#define RXTIMEOUT       0x0100
#define RXFIFOFULL      0x0200
#define RXFIFOTHOLD     0x0400
#define RXFIFONE        0x0800
#define RXFRAMERR       0x1000
#define RXPARERR        0x2000
#define RXBRK           0x4000
    uint16        intMask;                /* Same Bit defs for Mask and status */

    uint16        Data;                   /* Write to TX, Read from RX */
                                          /* bits 11:8 are BRK,PAR,FRM errors */
    uint16        unused2;

    uint32        unused3;
    uint32        unused4;
#else
    byte          unused0;
    byte          control;
#define BRGEN           0x80    /* Control register bit defs */
#define TXEN            0x40
#define RXEN            0x20
#define LOOPBK          0x10
#define TXPARITYEN      0x08
#define TXPARITYEVEN    0x04
#define RXPARITYEN      0x02
#define RXPARITYEVEN    0x01

    byte          config;
#define XMITBREAK       0x40
#define BITS5SYM        0x00
#define BITS6SYM        0x10
#define BITS7SYM        0x20
#define BITS8SYM        0x30
#define ONESTOP         0x07
#define TWOSTOP         0x0f
    /* 4-LSBS represent STOP bits/char
     * in 1/8 bit-time intervals.  Zero
     * represents 1/8 stop bit interval.
     * Fifteen represents 2 stop bits.
     */
    byte          fifoctl;
#define RSTTXFIFOS      0x80
#define RSTRXFIFOS      0x40
    /* 5-bit TimeoutCnt is in low bits of this register.
     *  This count represents the number of characters
     *  idle times before setting receive Irq when below threshold
     */
    uint32        baudword;
    /* When divide SysClk/2/(1+baudword) we should get 32*bit-rate
     */

    byte          txf_levl;       /* Read-only fifo depth */
    byte          rxf_levl;       /* Read-only fifo depth */
    byte          fifocfg;        /* Upper 4-bits are TxThresh, Lower are
                                   *      RxThreshold.  Irq can be asserted
                                   *      when rx fifo> thresh, txfifo<thresh
                                   */
    byte          prog_out;       /* Set value of DTR (Bit0), RTS (Bit1)
                                   *  if these bits are also enabled to GPIO_o
                                   */
#define DTREN   0x01
#define RTSEN   0x02

    byte          unused1;
    byte          DeltaIPEdgeNoSense;     /* Low 4-bits, set corr bit to 1 to
                                           * detect irq on rising AND falling
                                           * edges for corresponding GPIO_i
                                           * if enabled (edge insensitive)
                                           */
    byte          DeltaIPConfig_Mask;     /* Upper 4 bits: 1 for posedge sense
                                           *      0 for negedge sense if
                                           *      not configured for edge
                                           *      insensitive (see above)
                                           * Lower 4 bits: Mask to enable change
                                           *  detection IRQ for corresponding
                                           *  GPIO_i
                                           */
    byte          DeltaIP_SyncIP;         /* Upper 4 bits show which bits
                                           *  have changed (may set IRQ).
                                           *  read automatically clears bit
                                           * Lower 4 bits are actual status
                                           */

    uint16        intMask;                /* Same Bit defs for Mask and status */
    uint16        intStatus;
#define DELTAIP         0x0001
#define TXUNDERR        0x0002
#define TXOVFERR        0x0004
#define TXFIFOTHOLD     0x0008
#define TXREADLATCH     0x0010
#define TXFIFOEMT       0x0020
#define RXUNDERR        0x0040
#define RXOVFERR        0x0080
#define RXTIMEOUT       0x0100
#define RXFIFOFULL      0x0200
#define RXFIFOTHOLD     0x0400
#define RXFIFONE        0x0800
#define RXFRAMERR       0x1000
#define RXPARERR        0x2000
#define RXBRK           0x4000

    uint16        unused2;
    uint16        Data;                   /* Write to TX, Read from RX */
                                          /* bits 11:8 are BRK,PAR,FRM errors */

    uint32        unused3;
    uint32        unused4;
#endif
} Uart;

#define UART ((volatile Uart * const) UART_BASE)

#define PERIPH_MUTEX ((volatile unsigned int * const) PERIPH_MUTEX_BASE)

/*
** High-Speed SPI Controller
*/

#define __mask(end, start)      (((1 << ((end - start) + 1)) - 1) << start)
typedef struct HsSpiControl {

    uint32    hs_spiGlobalCtrl;   // 0x0000
#define HS_SPI_MOSI_IDLE        (1 << 18)
#define HS_SPI_CLK_POLARITY      (1 << 17)
#define HS_SPI_CLK_GATE_SSOFF       (1 << 16)
#define HS_SPI_PLL_CLK_CTRL     (8)
#define HS_SPI_PLL_CLK_CTRL_MASK    __mask(15, HS_SPI_PLL_CLK_CTRL)
#define HS_SPI_SS_POLARITY      (0)
#define HS_SPI_SS_POLARITY_MASK     __mask(7, HS_SPI_SS_POLARITY)

    uint32    hs_spiExtTrigCtrl;  // 0x0004
#define HS_SPI_TRIG_RAW_STATE   (24)
#define HS_SPI_TRIG_RAW_STATE_MASK  __mask(31, HS_SPI_TRIG_RAW_STATE)
#define HS_SPI_TRIG_LATCHED     (16)
#define HS_SPI_TRIG_LATCHED_MASK    __mask(23, HS_SPI_TRIG_LATCHED)
#define HS_SPI_TRIG_SENSE       (8)
#define HS_SPI_TRIG_SENSE_MASK      __mask(15, HS_SPI_TRIG_SENSE)
#define HS_SPI_TRIG_TYPE        (0)
#define HS_SPI_TRIG_TYPE_MASK       __mask(7, HS_SPI_TRIG_TYPE)
#define HS_SPI_TRIG_TYPE_EDGE       (0)
#define HS_SPI_TRIG_TYPE_LEVEL      (1)

    uint32    hs_spiIntStatus;    // 0x0008
#define HS_SPI_IRQ_PING1_USER       (28)
#define HS_SPI_IRQ_PING1_USER_MASK  __mask(31, HS_SPI_IRQ_PING1_USER)
#define HS_SPI_IRQ_PING0_USER       (24)
#define HS_SPI_IRQ_PING0_USER_MASK  __mask(27, HS_SPI_IRQ_PING0_USER)

#define HS_SPI_IRQ_PING1_CTRL_INV   (1 << 12)
#define HS_SPI_IRQ_PING1_POLL_TOUT  (1 << 11)
#define HS_SPI_IRQ_PING1_TX_UNDER   (1 << 10)
#define HS_SPI_IRQ_PING1_RX_OVER    (1 << 9)
#define HS_SPI_IRQ_PING1_CMD_DONE   (1 << 8)

#define HS_SPI_IRQ_PING0_CTRL_INV   (1 << 4)
#define HS_SPI_IRQ_PING0_POLL_TOUT  (1 << 3)
#define HS_SPI_IRQ_PING0_TX_UNDER   (1 << 2)
#define HS_SPI_IRQ_PING0_RX_OVER    (1 << 1)
#define HS_SPI_IRQ_PING0_CMD_DONE   (1 << 0)

    uint32    hs_spiIntStatusMasked;  // 0x000C
#define HS_SPI_IRQSM__PING1_USER    (28)
#define HS_SPI_IRQSM__PING1_USER_MASK   __mask(31, HS_SPI_IRQSM__PING1_USER)
#define HS_SPI_IRQSM__PING0_USER    (24)
#define HS_SPI_IRQSM__PING0_USER_MASK   __mask(27, HS_SPI_IRQSM__PING0_USER)

#define HS_SPI_IRQSM__PING1_CTRL_INV    (1 << 12)
#define HS_SPI_IRQSM__PING1_POLL_TOUT   (1 << 11)
#define HS_SPI_IRQSM__PING1_TX_UNDER    (1 << 10)
#define HS_SPI_IRQSM__PING1_RX_OVER (1 << 9)
#define HS_SPI_IRQSM__PING1_CMD_DONE    (1 << 8)

#define HS_SPI_IRQSM__PING0_CTRL_INV    (1 << 4)
#define HS_SPI_IRQSM__PING0_POLL_TOUT   (1 << 3)
#define HS_SPI_IRQSM__PING0_TX_UNDER    (1 << 2)
#define HS_SPI_IRQSM__PING0_RX_OVER     (1 << 1)
#define HS_SPI_IRQSM__PING0_CMD_DONE    (1 << 0)

    uint32    hs_spiIntMask;      // 0x0010
#define HS_SPI_IRQM_PING1_USER      (28)
#define HS_SPI_IRQM_PING1_USER_MASK __mask(31, HS_SPI_IRQM_PING1_USER)
#define HS_SPI_IRQM_PING0_USER      (24)
#define HS_SPI_IRQM_PING0_USER_MASK __mask(27, HS_SPI_IRQM_PING0_USER)

#define HS_SPI_IRQM_PING1_CTRL_INV  (1 << 12)
#define HS_SPI_IRQM_PING1_POLL_TOUT (1 << 11)
#define HS_SPI_IRQM_PING1_TX_UNDER  (1 << 10)
#define HS_SPI_IRQM_PING1_RX_OVER   (1 << 9)
#define HS_SPI_IRQM_PING1_CMD_DONE  (1 << 8)

#define HS_SPI_IRQM_PING0_CTRL_INV  (1 << 4)
#define HS_SPI_IRQM_PING0_POLL_TOUT (1 << 3)
#define HS_SPI_IRQM_PING0_TX_UNDER  (1 << 2)
#define HS_SPI_IRQM_PING0_RX_OVER   (1 << 1)
#define HS_SPI_IRQM_PING0_CMD_DONE  (1 << 0)

#define HS_SPI_INTR_CLEAR_ALL       (0xFF001F1F)

    uint32    hs_spiFlashCtrl;    // 0x0014
#define HS_SPI_FCTRL_MB_ENABLE      (23)
#define HS_SPI_FCTRL_SS_NUM         (20)
#define HS_SPI_FCTRL_SS_NUM_MASK    __mask(22, HS_SPI_FCTRL_SS_NUM)
#define HS_SPI_FCTRL_PROFILE_NUM    (16)
#define HS_SPI_FCTRL_PROFILE_NUM_MASK   __mask(18, HS_SPI_FCTRL_PROFILE_NUM)
#define HS_SPI_FCTRL_DUMMY_BYTES    (10)
#define HS_SPI_FCTRL_DUMMY_BYTES_MASK   __mask(11, HS_SPI_FCTRL_DUMMY_BYTES)
#define HS_SPI_FCTRL_ADDR_BYTES     (8)
#define HS_SPI_FCTRL_ADDR_BYTES_MASK    __mask(9, HS_SPI_FCTRL_ADDR_BYTES)
#define HS_SPI_FCTRL_ADDR_BYTES_2   (0)
#define HS_SPI_FCTRL_ADDR_BYTES_3   (1)
#define HS_SPI_FCTRL_ADDR_BYTES_4   (2)
#define HS_SPI_FCTRL_READ_OPCODE    (0)
#define HS_SPI_FCTRL_READ_OPCODE_MASK   __mask(7, HS_SPI_FCTRL_READ_OPCODE)

    uint32    hs_spiFlashAddrBase;    // 0x0018

    char      fill0[0x80 - 0x18];

    uint32    hs_spiPP_0_Cmd;     // 0x0080
#define HS_SPI_PP_SS_NUM        (12)
#define HS_SPI_PP_SS_NUM_MASK       __mask(14, HS_SPI_PP_SS_NUM)
#define HS_SPI_PP_PROFILE_NUM       (8)
#define HS_SPI_PP_PROFILE_NUM_MASK  __mask(10, HS_SPI_PP_PROFILE_NUM)

} HsSpiControl;

typedef struct HsSpiPingPong {

    uint32 command;
#define HS_SPI_SS_NUM (12)
#define HS_SPI_PROFILE_NUM (8)
#define HS_SPI_TRIGGER_NUM (4)
#define HS_SPI_COMMAND_VALUE (0)
#define HS_SPI_COMMAND_NOOP (0)
#define HS_SPI_COMMAND_START_NOW (1)
#define HS_SPI_COMMAND_START_TRIGGER (2)
#define HS_SPI_COMMAND_HALT (3)
#define HS_SPI_COMMAND_FLUSH (4)

    uint32 status;
#define HS_SPI_ERROR_BYTE_OFFSET (16)
#define HS_SPI_WAIT_FOR_TRIGGER (2)
#define HS_SPI_SOURCE_BUSY (1)
#define HS_SPI_SOURCE_GNT (0)

    uint32 fifo_status;
    uint32 control;

} HsSpiPingPong;

typedef struct HsSpiProfile {

    uint32 clk_ctrl;
#define HS_SPI_ACCUM_RST_ON_LOOP (15)
#define HS_SPI_SPI_CLK_2X_SEL (14)
#define HS_SPI_FREQ_CTRL_WORD (0)

    uint32 signal_ctrl;
#define HS_SPI_ASYNC_INPUT_PATH (1 << 16)
#define HS_SPI_LAUNCH_RISING    (1 << 13)
#define HS_SPI_LATCH_RISING     (1 << 12)

    uint32 mode_ctrl;
#define HS_SPI_PREPENDBYTE_CNT (24)
#define HS_SPI_MODE_ONE_WIRE (20)
#define HS_SPI_MULTIDATA_WR_SIZE (18)
#define HS_SPI_MULTIDATA_RD_SIZE (16)
#define HS_SPI_MULTIDATA_WR_STRT (12)
#define HS_SPI_MULTIDATA_RD_STRT (8)
#define HS_SPI_FILLBYTE (0)

    uint32 polling_config;
    uint32 polling_and_mask;
    uint32 polling_compare;
    uint32 polling_timeout;
    uint32 reserved;

} HsSpiProfile;

#define HS_SPI_OP_CODE 13
#define HS_SPI_OP_SLEEP (0)
#define HS_SPI_OP_READ_WRITE (1)
#define HS_SPI_OP_WRITE (2)
#define HS_SPI_OP_READ (3)
#define HS_SPI_OP_SETIRQ (4)

#define HS_SPI ((volatile HsSpiControl * const) HSSPIM_BASE)
#define HS_SPI_PINGPONG0 ((volatile HsSpiPingPong * const) (HSSPIM_BASE+0x80))
#define HS_SPI_PINGPONG1 ((volatile HsSpiPingPong * const) (HSSPIM_BASE+0xc0))
#define HS_SPI_PROFILES ((volatile HsSpiProfile * const) (HSSPIM_BASE+0x100))
#define HS_SPI_FIFO0 ((volatile uint8 * const) (HSSPIM_BASE+0x200))
#define HS_SPI_FIFO1 ((volatile uint8 * const) (HSSPIM_BASE+0x400))



#define DDR_UBUS_ADDRESS_BASE         0
#ifdef __cplusplus
}
#endif

#endif

