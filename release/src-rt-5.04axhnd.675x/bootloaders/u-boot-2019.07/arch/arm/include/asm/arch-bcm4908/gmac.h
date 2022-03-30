/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Corporation.
 */

#ifndef _BCM_SF2_ETH_GMAC_H_
#define _BCM_SF2_ETH_GMAC_H_

/*
 * GMAC(UNIMAC) module 
 */

typedef union MibCtrlReg_s {
    uint32_t word;
    struct {
        uint32_t    clrMib:  1;
        uint32_t    unused: 31;
    };
} MibCtrlReg_t;

typedef union MibMaxPktSizeReg_s {
    uint32_t word;
    struct {
        uint32_t    max_pkt_size: 14;
        uint32_t    unused      : 18;
    };
} MibMaxPktSizeReg_t;

typedef union RxBpThreshReg_s {
    uint32_t word;
    struct {
        uint32_t    rx_thresh : 11;
        uint32_t    unused    : 21;
    };
} RxBpThreshReg_t;

typedef union RxFlowCtrlReg_s {
    uint32_t word;
    struct {
        uint32_t    fc_en     :  1;
        uint32_t    pause_en  :  1;
        uint32_t    unused    : 20;
    };
} RxFlowCtrlReg_t;

typedef union BpForceReg_s {
    uint32_t word;
    struct {
        uint32_t    force :  1;
        uint32_t    unused: 31;
    };
} BpForceReg_t;

typedef union IrqEnableReg_s {
    uint32_t word;
    struct {
        uint32_t    ovfl  :  1;
        uint32_t    unused: 31;
    };
} IrqEnableReg_t;

typedef union IrqStatusReg_s {
    uint32_t word;
    struct {
        uint32_t    ovfl  :  1;
        uint32_t    unused: 31;
    };
} IrqStatusReg_t;

typedef union GmacStatusReg_s {
    uint32_t  word;
    struct {
        uint32_t  eth_speed   : 2;
#define GMAC_STATUS_SPEED_10        0
#define GMAC_STATUS_SPEED_100       1
#define GMAC_STATUS_SPEED_1000      2
        uint32_t  hd          : 1;
        uint32_t  auto_cfg_en : 1;
        uint32_t  link_up     : 1;
        uint32_t  unused      :27;
    };
} GmacStatusReg_t;

typedef union FlushReg_s {
    uint32_t  word;
    struct {
        uint32_t  rxfifo_flush : 1;
        uint32_t  txfifo_flush : 1;
        uint32_t  unused       :30;
    };
} FlushReg_t;

typedef union DmaRxStatusSelReg_s {
    uint32_t  word;
    struct {
        uint32_t  rx_err    : 1;
        uint32_t  crc_err   : 1;
        uint32_t  mcast_det : 1;
        uint32_t  bcast_det : 1;
        uint32_t  ctrl_frm  : 1;
        uint32_t  vlan      : 1;
        uint32_t  ucast_det : 1;
        uint32_t  frm_trunc : 1;
        uint32_t  runt_det  : 1;
        uint32_t  unused    :23;
    };
} DmaRxStatusSelReg_t;

typedef union DmaRxOkToSendCountReg_s {
    uint32_t  word;
    struct {
        uint32_t  ok_to_send_count : 4;
        uint32_t  unused           :28;
    };
} DmaRxOkToSendCountReg_t;

typedef struct GmacIntf {
/*0x00*/    uint32_t                Control; 
/*0x04*/    MibCtrlReg_t            MibCtrl;
/*0x08*/    uint32_t                RxErrMask; 
/*0x0C*/    MibMaxPktSizeReg_t      MibMaxPktSize;
/*0x10*/    uint32_t                reserved1[3];
/*0x1C*/    uint32_t                DiagOut; 
/*0x20*/    uint32_t                EnableDropPkt;
/*0x24*/    IrqEnableReg_t          IrqEnable;
/*0x28*/    GmacStatusReg_t         GmacStatus;
/*0x2C*/    IrqStatusReg_t          IrqStatus; 
/*0x30*/    uint32_t                OverFlowCounter; 
/*0x34*/    FlushReg_t              Flush;
/*0x38*/    uint32_t                RsvSelect;  
/*0x3C*/    BpForceReg_t            BpForce;
/*0x40*/    DmaRxOkToSendCountReg_t DmaRxOkToSendCount;
/*0x44*/    uint32_t                TxCrcCtrl;  
} GmacIntf;

typedef union CmdReg_s {
    uint32_t word;
    struct {
        uint32_t tx_ena        : 1;   /* bit  0 */
        uint32_t rx_ena        : 1;   /* bit  1 */
        uint32_t eth_speed     : 2;   /* bit 3:2 */
#define CMD_ETH_SPEED_10            0
#define CMD_ETH_SPEED_100           1
#define CMD_ETH_SPEED_1000          2
#define CMD_ETH_SPEED_2500          3
        uint32_t promis_en     : 1;   /* bit  4 */
        uint32_t pad_rem_en    : 1;   /* bit  5 */
        uint32_t crc_fwd       : 1;   /* bit  6 */
        uint32_t pause_fwd     : 1;   /* bit  7 */
        uint32_t rx_pause_ign  : 1;   /* bit  8 */
        uint32_t tx_addr_ins   : 1;   /* bit  9 */
        uint32_t hd_ena        : 1;   /* bit 10 */
        uint32_t unused0       : 2;   /* bit 12:11 */
        uint32_t sw_reset      : 1;   /* bit 13 */
        uint32_t unused1       : 1;   /* bit 14 */
        uint32_t lcl_loop_ena  : 1;   /* bit 15 */
        uint32_t unused2       : 6;   /* bit 21:16 */
        uint32_t ena_ext_cfg   : 1;   /* bit 22 */
        uint32_t ctrl_frm_ena  : 1;   /* bit 23 */
        uint32_t len_chk_dis   : 1;   /* bit 24 */
        uint32_t rmt_loop_ena  : 1;   /* bit 25 */
        uint32_t rx_err_disc   : 1;   /* bit 26 */
        uint32_t prbl_ena      : 1;   /* bit 27 */
        uint32_t tx_pause_ign  : 1;   /* bit 28 */
        uint32_t txrx_en_cfg   : 1;   /* bit 29 */
        uint32_t runt_filt_dis : 1;   /* bit 30 */
        uint32_t unused3       : 1;   /* bit 31 */
    };
} CmdReg_t;

typedef union FrmLenReg_s {
    uint32_t  word;
    struct {
        uint32_t frm_len : 14;   /* bit 13:0 */
        uint32_t unused  : 18;   
    };
} FrmLenReg_t;

typedef union PauseQuantaReg_s {
    uint32_t  word;
    struct {
        uint32_t pause_quanta : 16;   /* bit 15:0 */
        uint32_t unused       : 16;   
    };
} PauseQuantaReg_t;

typedef union ModeReg_s {
    uint32_t  word;
    struct {
        uint32_t mac_speed    : 2;   /* bit 1:0 */
        uint32_t mac_dplx     : 1;   /* bit  2 */
        uint32_t mac_rx_pause : 1;   /* bit  3 */
        uint32_t mac_tx_pause : 1;   /* bit  4 */
        uint32_t mac_link_up  : 1;   /* bit  5 */
        uint32_t unused       : 26;   
    };
} ModeReg_t;

typedef union FrmTagReg_s {
    uint32_t  word;
    struct {
        uint32_t tag     : 16;    /* bit 15:0 */
        uint32_t tpid_en :  1;    /* bit 16 */
        uint32_t unused  : 15;   
    };
} FrmTagReg_t;

typedef union TxIpgLenReg_s {
    uint32_t  word;
    struct {
        uint32_t tx_ipg_len     : 7;  /* bit 6:0 */
        uint32_t unused2        : 1;  /* bit 7 */
        uint32_t tx_min_pkt_len : 7;  /* bit 14:8 */
        uint32_t unused1        : 17; /* bit 31:15 */
    };

} TxIpgLenReg_t;

typedef union RxIpgInvReg_s {
    uint32_t  word;
    struct {
        uint32_t rx_ipg_inv : 1;  /* bit 0 */
        uint32_t unused     :31;   
    };
} RxIpgInvReg_t;

typedef union RepPauseCtrlReg_s {
    uint32_t  word;
    struct {
        uint32_t pause_timer :17; /* bit 16:0 */
        uint32_t pause_en    : 1; /* bit 17 */
        uint32_t unused      :14;   
    };
} RepPauseCtrlReg_t;

typedef union TxFifoFlushReg_s {
    uint32_t  word;
    struct {
        uint32_t tx_flush : 1; /* bit 0 */
        uint32_t unused   :31;   
    };
} TxFifoFlushReg_t;

typedef struct RxFifoStatusReg_s {
    uint32_t  word;
    struct {
        uint32_t rxfifo_underrun : 1; /* bit 0 */
        uint32_t rxfifo_overrun  : 1; /* bit 1 */
        uint32_t unused          :30;   
    };
} RxFifoStatusReg_t;

typedef union TxFifoStatusReg_s {
    uint32_t word;
    struct {
        uint32_t txfifo_underrun : 1; /* bit 0 */
        uint32_t txfifo_overrun  : 1; /* bit 1 */
        uint32_t unused          :30;   
    };
} TxFifoStatusReg_t;


typedef struct GmacMac {
    uint32_t UmacDummy;             /* 0x00 */
    uint32_t HdBkpCntl;             /* 0x04 */
    CmdReg_t Cmd;                   /* 0x08 */
    uint32_t Mac0;                  /* 0x0c */
    uint32_t Mac1;                  /* 0x10 */
    FrmLenReg_t FrmLen;             /* 0x14 */
    PauseQuantaReg_t PauseQuanta;   /* 0x18 */
    uint32_t unused1[9];            /* 0x1c - 0x3c */
    uint32_t SfdOffset;             /* 0x40 */
    ModeReg_t Mode;                 /* 0x44 */
    FrmTagReg_t FrmTag0;            /* 0x48 */
    FrmTagReg_t FrmTag1;            /* 0x4c */
    uint32_t unused2[3];            /* 0x50 - 0x58 */
    TxIpgLenReg_t TxIpgLen;         /* 0x5c */
    uint32_t unused3[6];            /* 0x60 - 0x74 */
    RxIpgInvReg_t RxIpgInv;         /* 0x78 */
    uint32_t unused4[165];          /* 0x7c - 0x30c */
    uint32_t MacsecProgTxCrc;       /* 0x310 */
    uint32_t MacsecCtrl;            /* 0x314 */
    uint32_t unused5[6];            /* 0x318 - 0x32c */
    RepPauseCtrlReg_t PauseCtrl;    /* 0x330 */
    TxFifoFlushReg_t TxFifoFlush;   /* 0x334 */
    RxFifoStatusReg_t RxFifoStatus; /* 0x338 */
    TxFifoStatusReg_t TxFifoStatus; /* 0x33c */
} GmacMac;


#define GMAC_IUDMA_MAX_CHANNELS          2
/*
** DMA Channel Configuration (1 .. 32)
*/
typedef struct DmaChannelCfg {
  uint32_t        cfg;                    /* (00) assorted configuration */
#define         DMA_ENABLE      0x00000001  /* set to enable channel */
#define         DMA_PKT_HALT    0x00000002  /* idle after an EOP flag is detected */
#define         DMA_BURST_HALT  0x00000004  /* idle after finish current memory burst */
  uint32_t        intStat;                /* (04) interrupts control and status */
  uint32_t        intMask;                /* (08) interrupts mask */
#define         DMA_BUFF_DONE   0x00000001  /* buffer done */
#define         DMA_DONE        0x00000002  /* packet xfer complete */
#define         DMA_NO_DESC     0x00000004  /* no valid descriptors */
#define         DMA_RX_ERROR    0x00000008  /* rxdma detect client protocol error */
  uint32_t        maxBurst;               /* (0C) max burst length permitted */
#define         DMA_DESCSIZE_SEL 0x00040000  /* DMA Descriptor Size Selection */
} DmaChannelCfg;

/*
** DMA State RAM (1 .. 16)
*/
typedef struct DmaStateRam {
  uint32_t        baseDescPtr;            /* (00) descriptor ring start address */
#define			RING_OFFSET_MASK 0x00001FFF
  uint32_t        state_data;             /* (04) state/bytes done/ring offset */
  uint32_t        desc_len_status;        /* (08) buffer descriptor status and len */
  uint32_t        desc_base_bufptr;       /* (0C) buffer descrpitor current processing */
} DmaStateRam;


/*
** DMA Registers
*/
typedef struct DmaRegs {
    uint32_t controller_cfg;              /* (00) controller configuration */
#define DMA_MASTER_EN           0x00000001
#define DMA_FLOWC_CH1_EN        0x00000002
#define DMA_FLOWC_CH3_EN        0x00000004

    // Flow control Ch1
    uint32_t flowctl_ch1_thresh_lo;           /* 004 */
    uint32_t flowctl_ch1_thresh_hi;           /* 008 */
    uint32_t flowctl_ch1_alloc;               /* 00c */
#define DMA_BUF_ALLOC_FORCE     0x80000000

    // Flow control Ch3
    uint32_t flowctl_ch3_thresh_lo;           /* 010 */
    uint32_t flowctl_ch3_thresh_hi;           /* 014 */
    uint32_t flowctl_ch3_alloc;               /* 018 */

    // Flow control Ch5
    uint32_t flowctl_ch5_thresh_lo;           /* 01C */
    uint32_t flowctl_ch5_thresh_hi;           /* 020 */
    uint32_t flowctl_ch5_alloc;               /* 024 */

    // Flow control Ch7
    uint32_t flowctl_ch7_thresh_lo;           /* 028 */
    uint32_t flowctl_ch7_thresh_hi;           /* 02C */
    uint32_t flowctl_ch7_alloc;               /* 030 */

    uint32_t ctrl_channel_reset;              /* 034 */
    uint32_t ctrl_channel_debug;              /* 038 */
    uint32_t reserved1;                       /* 03C */
    uint32_t ctrl_global_interrupt_status;    /* 040 */
    uint32_t ctrl_global_interrupt_mask;      /* 044 */

    // Unused words
    uint8_t reserved2[0x200-0x48];

    // Per channel registers/state ram
    DmaChannelCfg chcfg[GMAC_IUDMA_MAX_CHANNELS];/* (0x200-0x21f) Channel configuration */
    // Unused words
    uint8_t reserved3[0x200-0x20];
    union {
        DmaStateRam     s[GMAC_IUDMA_MAX_CHANNELS];
        uint32_t          u32[4 * GMAC_IUDMA_MAX_CHANNELS];
    } stram;                                /* (400-5FF) state ram */
} DmaRegs;

/*
** DMA Buffer
*/
typedef struct DmaDesc {
  union {
    struct {
        uint16_t        status;                 /* buffer status */
#define          DMA_OWN                0x8000  /* cleared by DMA, set by SW */
#define          DMA_EOP                0x4000  /* last buffer in packet */
#define          DMA_SOP                0x2000  /* first buffer in packet */
#define          DMA_WRAP               0x1000  /* */
#define          DMA_PRIO               0x0C00  /* Prio for Tx */
#define          DMA_APPEND_BRCM_TAG    0x0200
#define          DMA_APPEND_CRC         0x0100
#define          USB_ZERO_PKT           (1<< 0) // Set to send zero length packet
        uint16_t        length;                 /* in bytes of data in buffer */
#define          DMA_DESC_USEFPM    0x8000
#define          DMA_DESC_MULTICAST 0x4000
#define          DMA_DESC_BUFLENGTH 0x0fff
    };
    uint32_t      word0;
  };
  uint32_t        address;                /* address of data */
} DmaDesc;

/*
** 16 Byte DMA Buffer
*/
typedef struct {
  union {
    struct {
        uint16_t        status;                 /* buffer status */
#define          DMA_OWN                0x8000  /* cleared by DMA, set by SW */
#define          DMA_EOP                0x4000  /* last buffer in packet */
#define          DMA_SOP                0x2000  /* first buffer in packet */
#define          DMA_WRAP               0x1000  /* */
#define          DMA_PRIO               0x0C00  /* Prio for Tx */
#define          DMA_APPEND_BRCM_TAG    0x0200
#define          DMA_APPEND_CRC         0x0100
#define          USB_ZERO_PKT           (1<< 0) // Set to send zero length packet
        uint16_t        length;                 /* in bytes of data in buffer */
#define          DMA_DESC_USEFPM    0x8000
#define          DMA_DESC_MULTICAST 0x4000
#define          DMA_DESC_BUFLENGTH 0x0fff
    };
    uint32_t      word0;
  };
  uint32_t        address;                 /* address of data */
  uint32_t        control;
#define         GEM_ID_MASK             0x001F
  uint32_t        reserved;
} DmaDesc16;

#endif /* _BCM_SF2_ETH_GMAC_H_ */
