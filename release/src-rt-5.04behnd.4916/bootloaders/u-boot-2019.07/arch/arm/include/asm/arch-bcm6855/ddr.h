/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _6855_DDR_H
#define _6855_DDR_H

#define MEMC_BASE   0x80180000  /* DDR IO Buf Control */

typedef struct AXIInterface {
   uint32_t CFG;                  /* 0x00 */
#define MC_AXIWIF_0_CFG_WRITE_REPLY_MODE_MASK                      0x00000100
#define MC_AXIWIF_0_CFG_WRITE_REPLY_MODE_ALIGN                     0
#define MC_AXIWIF_0_CFG_WRITE_REPLY_MODE_BITS                      1
#define MC_AXIWIF_0_CFG_WRITE_REPLY_MODE_SHIFT                     8
#define MC_AXIWIF_0_CFG_WRITE_REPLY_MODE_DEFAULT                   0x00000000

#define MC_AXIWIF_0_CFG_CPU_WRITE_IN_TRANSIT_MASK                  0x0000fc00
#define MC_AXIWIF_0_CFG_CPU_WRITE_IN_TRANSIT_ALIGN                 0
#define MC_AXIWIF_0_CFG_CPU_WRITE_IN_TRANSIT_BITS                  6
#define MC_AXIWIF_0_CFG_CPU_WRITE_IN_TRANSIT_SHIFT                 10
#define MC_AXIWIF_0_CFG_CPU_WRITE_IN_TRANSIT_DEFAULT               0x00000008

#define MC_AXIWIF_0_CFG_CPQ_WRITE_IN_TRANSIT_MASK                  0x003f0000
#define MC_AXIWIF_0_CFG_CPQ_WRITE_IN_TRANSIT_ALIGN                 0
#define MC_AXIWIF_0_CFG_CPQ_WRITE_IN_TRANSIT_BITS                  6
#define MC_AXIWIF_0_CFG_CPQ_WRITE_IN_TRANSIT_SHIFT                 16
#define MC_AXIWIF_0_CFG_CPQ_WRITE_IN_TRANSIT_DEFAULT               0x00000008

#define MC_AXIWIF_0_CFG_WRITE_ACK_MODE_MASK                        0x00000200
#define MC_AXIWIF_0_CFG_WRITE_ACK_MODE_ALIGN                       0
#define MC_AXIWIF_0_CFG_WRITE_ACK_MODE_BITS                        1
#define MC_AXIWIF_0_CFG_WRITE_ACK_MODE_SHIFT                       9
#define MC_AXIWIF_0_CFG_WRITE_ACK_MODE_DEFAULT                     0x00000000

#define MC_AXIWIF_0_CFG_CPQ_WR_FULL_THRESH_MASK                    0xf8000000
#define MC_AXIWIF_0_CFG_CPQ_WR_FULL_THRESH_ALIGN                   0
#define MC_AXIWIF_0_CFG_CPQ_WR_FULL_THRESH_BITS                    5
#define MC_AXIWIF_0_CFG_CPQ_WR_FULL_THRESH_SHIFT                   27
#define MC_AXIWIF_0_CFG_CPQ_WR_FULL_THRESH_DEFAULT                 0x00000008

#define MC_AXIWIF_0_CFG_CPU_WR_FULL_THRESH_MASK                    0x07c00000
#define MC_AXIWIF_0_CFG_CPU_WR_FULL_THRESH_ALIGN                   0
#define MC_AXIWIF_0_CFG_CPU_WR_FULL_THRESH_BITS                    5
#define MC_AXIWIF_0_CFG_CPU_WR_FULL_THRESH_SHIFT                   22
#define MC_AXIWIF_0_CFG_CPU_WR_FULL_THRESH_DEFAULT                 0x00000008

#define MC_AXIWIF_0_CFG_WR_COM_THD_MASK                            0x000000f0
#define MC_AXIWIF_0_CFG_WR_COM_THD_ALIGN                           0
#define MC_AXIWIF_0_CFG_WR_COM_THD_BITS                            4
#define MC_AXIWIF_0_CFG_WR_COM_THD_SHIFT                           4
#define MC_AXIWIF_0_CFG_WR_COM_THD_DEFAULT                         0x0000000f

#define MC_AXIWIF_0_CFG_WR_REQ_THD_MASK                            0x0000000f
#define MC_AXIWIF_0_CFG_WR_REQ_THD_ALIGN                           0
#define MC_AXIWIF_0_CFG_WR_REQ_THD_BITS                            4
#define MC_AXIWIF_0_CFG_WR_REQ_THD_SHIFT                           0
#define MC_AXIWIF_0_CFG_WR_REQ_THD_DEFAULT                         0x0000000f
   uint32_t REP_ARB_MODE;         /* 0x04 */
#define MC_AXIWIF_0_REP_ARB_MODE_RD_FIFO_MODE_MASK                 0x00000004
#define MC_AXIWIF_0_REP_ARB_MODE_RD_FIFO_MODE_ALIGN                0
#define MC_AXIWIF_0_REP_ARB_MODE_RD_FIFO_MODE_BITS                 1
#define MC_AXIWIF_0_REP_ARB_MODE_RD_FIFO_MODE_SHIFT                2
#define MC_AXIWIF_0_REP_ARB_MODE_RD_FIFO_MODE_DEFAULT              0x00000000

#define MC_AXIWIF_0_REP_ARB_MODE_RD_PACKET_MODE_MASK               0x00000002
#define MC_AXIWIF_0_REP_ARB_MODE_RD_PACKET_MODE_ALIGN              0
#define MC_AXIWIF_0_REP_ARB_MODE_RD_PACKET_MODE_BITS               1
#define MC_AXIWIF_0_REP_ARB_MODE_RD_PACKET_MODE_SHIFT              1
#define MC_AXIWIF_0_REP_ARB_MODE_RD_PACKET_MODE_DEFAULT            0x00000000

#define MC_AXIWIF_0_REP_ARB_MODE_BRESP_PIPELINE_EN_MASK            0x00000020
#define MC_AXIWIF_0_REP_ARB_MODE_BRESP_PIPELINE_EN_ALIGN           0
#define MC_AXIWIF_0_REP_ARB_MODE_BRESP_PIPELINE_EN_BITS            1
#define MC_AXIWIF_0_REP_ARB_MODE_BRESP_PIPELINE_EN_SHIFT           5
#define MC_AXIWIF_0_REP_ARB_MODE_BRESP_PIPELINE_EN_DEFAULT         0x00000000

#define MC_AXIWIF_0_REP_ARB_MODE_DIS_ADDR_HAZARD_CHK_MASK          0x00000010
#define MC_AXIWIF_0_REP_ARB_MODE_DIS_ADDR_HAZARD_CHK_ALIGN         0
#define MC_AXIWIF_0_REP_ARB_MODE_DIS_ADDR_HAZARD_CHK_BITS          1
#define MC_AXIWIF_0_REP_ARB_MODE_DIS_ADDR_HAZARD_CHK_SHIFT         4
#define MC_AXIWIF_0_REP_ARB_MODE_DIS_ADDR_HAZARD_CHK_DEFAULT       0x00000000

#define MC_AXIWIF_0_REP_ARB_MODE_RRESP_ERR_MODE_MASK               0x00000008
#define MC_AXIWIF_0_REP_ARB_MODE_RRESP_ERR_MODE_ALIGN              0
#define MC_AXIWIF_0_REP_ARB_MODE_RRESP_ERR_MODE_BITS               1
#define MC_AXIWIF_0_REP_ARB_MODE_RRESP_ERR_MODE_SHIFT              3
#define MC_AXIWIF_0_REP_ARB_MODE_RRESP_ERR_MODE_DEFAULT            0x00000000

#define MC_AXIWIF_0_REP_ARB_MODE_BRESP_PIPELINE_LEN_MASK           0x000003c0
#define MC_AXIWIF_0_REP_ARB_MODE_BRESP_PIPELINE_LEN_ALIGN          0
#define MC_AXIWIF_0_REP_ARB_MODE_BRESP_PIPELINE_LEN_BITS           4
#define MC_AXIWIF_0_REP_ARB_MODE_BRESP_PIPELINE_LEN_SHIFT          6
#define MC_AXIWIF_0_REP_ARB_MODE_BRESP_PIPELINE_LEN_DEFAULT        0x00000007

#define MC_AXIWIF_0_REP_ARB_MODE_FIFO_MODE_MASK                    0x00000001
#define MC_AXIWIF_0_REP_ARB_MODE_FIFO_MODE_ALIGN                   0
#define MC_AXIWIF_0_REP_ARB_MODE_FIFO_MODE_BITS                    1
#define MC_AXIWIF_0_REP_ARB_MODE_FIFO_MODE_SHIFT                   0
#define MC_AXIWIF_0_REP_ARB_MODE_FIFO_MODE_DEFAULT                 0x00000000
   uint32_t QUEUE_CFG;            /* 0x08 */
#define MC_AXIWIF_0_QUEUE_CFG_FULL_THRESH_QID_0_MASK               0x0000001f
#define MC_AXIWIF_0_QUEUE_CFG_FULL_THRESH_QID_0_ALIGN              0
#define MC_AXIWIF_0_QUEUE_CFG_FULL_THRESH_QID_0_BITS               5
#define MC_AXIWIF_0_QUEUE_CFG_FULL_THRESH_QID_0_SHIFT              0
#define MC_AXIWIF_0_QUEUE_CFG_FULL_THRESH_QID_0_DEFAULT            0x00000010

#define MC_AXIWIF_0_QUEUE_CFG_FULL_THRESH_QID_1_MASK               0x00001f00
#define MC_AXIWIF_0_QUEUE_CFG_FULL_THRESH_QID_1_ALIGN              0
#define MC_AXIWIF_0_QUEUE_CFG_FULL_THRESH_QID_1_BITS               5
#define MC_AXIWIF_0_QUEUE_CFG_FULL_THRESH_QID_1_SHIFT              8
#define MC_AXIWIF_0_QUEUE_CFG_FULL_THRESH_QID_1_DEFAULT            0x00000010

#define MC_AXIWIF_0_QUEUE_CFG_FULL_THRESH_QID_2_MASK               0x001f0000
#define MC_AXIWIF_0_QUEUE_CFG_FULL_THRESH_QID_2_ALIGN              0
#define MC_AXIWIF_0_QUEUE_CFG_FULL_THRESH_QID_2_BITS               5
#define MC_AXIWIF_0_QUEUE_CFG_FULL_THRESH_QID_2_SHIFT              16
#define MC_AXIWIF_0_QUEUE_CFG_FULL_THRESH_QID_2_DEFAULT            0x00000010

#define MC_AXIWIF_0_QUEUE_CFG_FULL_THRESH_QID_3_MASK               0x1f000000
#define MC_AXIWIF_0_QUEUE_CFG_FULL_THRESH_QID_3_ALIGN              0
#define MC_AXIWIF_0_QUEUE_CFG_FULL_THRESH_QID_3_BITS               5
#define MC_AXIWIF_0_QUEUE_CFG_FULL_THRESH_QID_3_SHIFT              24
#define MC_AXIWIF_0_QUEUE_CFG_FULL_THRESH_QID_3_DEFAULT            0x00000010
   uint32_t ESRCID_CFG;           /* 0x0c */
   uint32_t SRC_QUEUE_CTRL[8];    /* 0x10 */
   uint32_t SRC_ID_4_QUEUE_CTRL;  /* 0x30 */
   uint32_t SCRATCH;              /* 0x34 */
   uint32_t DEBUG[2];             /* 0x38 */
} AXIInterface;

typedef struct EDISEngine {
   uint32_t REV_ID;             /* 0x00 */
   uint32_t CTRL_TRIG;          /* 0x04 */
   uint32_t CTRL_MODE;          /* 0x08 */
   uint32_t CTRL_SIZE;          /* 0x0c */
   uint32_t CTRL_ADDR_START;    /* 0x10 */
   uint32_t CTRL_ADDR_START_EXT;/* 0x14 */
   uint32_t CTRL_ADDR_END;      /* 0x18 */
   uint32_t CTRL_ADDR_END_EXT;  /* 0x1c */
   uint32_t CTRL_WRITE_MASKS;   /* 0x20 */
   uint32_t CTRL_INT_ENABLES;   /* 0x24 */
   uint32_t unused0[6];         /* 0x28-0x3f */
   uint32_t STAT_MAIN;          /* 0x40 */
   uint32_t STAT_WORDS_WRITTEN; /* 0x44 */
   uint32_t STAT_WORDS_READ;    /* 0x48 */
   uint32_t STAT_ERROR_COUNT;   /* 0x4c */
   uint32_t STAT_ERROR_BITS;    /* 0x50 */
   uint32_t STAT_ADDR_LAST;     /* 0x54 */
   uint32_t STAT_ADDR_LAST_EXT; /* 0x58 */
   uint32_t STAT_CLOCK_CYCLES;  /* 0x5c */
   uint32_t unused1[7];         /* 0x60-0x7b */
   uint32_t STAT_DEBUG;         /* 0x7c */
   uint32_t STAT_DATA_PORT[8];  /* 0x80-0x9c */
   uint32_t GEN_LFSR_STATE[4];  /* 0xa0-0xac */
   uint32_t GEN_CLOCK;          /* 0xb0 */
   uint32_t GEN_PATTERN;        /* 0xb4 */
   uint32_t unused2[2];         /* 0xb8-0xbf */
   uint32_t BYTELANE_0_CTRL_LO; /* 0xc0 */
   uint32_t BYTELANE_0_CTRL_HI; /* 0xc4 */
   uint32_t BYTELANE_1_CTRL_LO; /* 0xc8 */
   uint32_t BYTELANE_1_CTRL_HI; /* 0xcc */
   uint32_t BYTELANE_2_CTRL_LO; /* 0xd0 */
   uint32_t BYTELANE_2_CTRL_HI; /* 0xd4 */
   uint32_t BYTELANE_3_CTRL_LO; /* 0xd8 */
   uint32_t BYTELANE_3_CTRL_HI; /* 0xdc */
   uint32_t BYTELANE_0_STAT_LO; /* 0xe0 */
   uint32_t BYTELANE_0_STAT_HI; /* 0xe4 */
   uint32_t BYTELANE_1_STAT_LO; /* 0xe8 */
   uint32_t BYTELANE_1_STAT_HI; /* 0xec */
   uint32_t BYTELANE_2_STAT_LO; /* 0xf0 */
   uint32_t BYTELANE_2_STAT_HI; /* 0xf4 */
   uint32_t BYTELANE_3_STAT_LO; /* 0xf8 */
   uint32_t BYTELANE_3_STAT_HI; /* 0xfc */
} EDISEngine;

typedef struct RangeCtrl {
   uint32_t CTRL;                /* 0x00 */
   uint32_t UBUS0_PORT;          /* 0x04 */
   uint32_t UBUS0_PORT_UPPER;    /* 0x08 */
   uint32_t BASE;                /* 0x0c */
   uint32_t BASE_UPPER;          /* 0x10 */
   uint32_t SECLEC_EN;           /* 0x14 */
} RangeCtrl;

typedef struct SecureRangeCheckers {
   uint32_t LOCK;            /* 0x00 */
   uint32_t LOG_INFO[3];     /* 0x04 - 0x0c */
   RangeCtrl RANGE_CTRL[8];/* 0x10 - 0xcf */
   uint32_t unused[12];      /* 0xd0 - 0xff */
} SecureRangeCheckers;

typedef struct DDRPhyControl {
   uint32_t PRIMARY_REVISION;    /* 0x00 */
   uint32_t SECONDARY_REVISION;  /* 0x04 */
   uint32_t FEATURE;             /* 0x08 */
   uint32_t PLL_STATUS;          /* 0x0c */
   uint32_t PLL_CONFIG;          /* 0x10 */
   uint32_t PLL_CONTROL1;        /* 0x14 */
   uint32_t PLL_CONTROL2;        /* 0x18 */
   uint32_t PLL_CONTROL3;        /* 0x1c */
   uint32_t PLL_DIVIDER;         /* 0x20 */
   uint32_t PLL_PRE_DIVIDER;     /* 0x24 */
   uint32_t PLL_SS_EN;           /* 0x28 */
   uint32_t PLL_SS_CFG;          /* 0x2c */
   uint32_t AUX_CONTROL;         /* 0x30 */
   uint32_t IDLE_PAD_CONTROL;    /* 0x34 */
   uint32_t IDLE_PAD_EN0;        /* 0x38 */
   uint32_t IDLE_PAD_EN1;        /* 0x3c */
   uint32_t DRIVE_PAD_CTL;       /* 0x40 */
   uint32_t DRIVE_PAD_CTL_2T;    /* 0x44 */
   uint32_t DRIVE_PAD_CTL_2T_A;  /* 0x48 */
   uint32_t STATIC_PAD_CTL;      /* 0x4c */
   uint32_t STATIC_PAD_CTL_2T;   /* 0x50 */
   uint32_t STATIC_PAD_CTL_2T_A; /* 0x54 */
   uint32_t RX_TRIM;             /* 0x58 */
   uint32_t DRAM_CFG;            /* 0x5c */
   uint32_t DRAM_TIMING2;        /* 0x60 */
   uint32_t DRAM_TIMING3;        /* 0x64 */
   uint32_t VDL_REGS[45];        /* 0x68-0x11b */
   uint32_t UPDATE_VDL;          /* 0x11c */
   uint32_t UPDATE_VDL_SNOOP1;   /* 0x120 */
   uint32_t UPDATE_VDL_SNOOP2;   /* 0x124 */
   uint32_t CMND_REG1;           /* 0x128 */
   uint32_t CMND_AUX_REG1;       /* 0x12c */
   uint32_t CMND_REG2;           /* 0x130 */
   uint32_t CMND_AUX_REG2;       /* 0x134 */
   uint32_t CMND_REG3;           /* 0x138 */
   uint32_t CMND_AUX_REG3;       /* 0x13c */
   uint32_t CMND_REG4;           /* 0x140 */
   uint32_t CMND_AUX_REG4;       /* 0x144 */
   uint32_t MODE_REG[8];         /* 0x148-167 */
   uint32_t ALERT_CLEAR;         /* 0x168 */
   uint32_t ALERT_STATUS;        /* 0x16c */
   uint32_t ALERT_DF1;           /* 0x170 */
   uint32_t WRITE_LEVEL_CTRL;    /* 0x174 */
   uint32_t WRITE_LEVEL_STATUS;  /* 0x178 */
   uint32_t READ_EN_CTRL;        /* 0x17c */
   uint32_t READ_EN_STATUS ;     /* 0x180 */
   uint32_t VIRT_VTT_CTRL;       /* 0x184 */
   uint32_t VIRT_VTT_STATUS;     /* 0x188 */
   uint32_t VIRT_VTT_CONNECTION; /* 0x18c */
   uint32_t VIRT_VTT_OVERRIDE;   /* 0x190 */
   uint32_t VREF_DAC_CTRL;       /* 0x194 */
   uint32_t STANDBY_CTRL;        /* 0x198 */
   uint32_t DEBUG_FREEZE_EN;     /* 0x19c */
   uint32_t DEBUG_MUX_CTRL;      /* 0x1a0 */
   uint32_t DFI_CTRL;            /* 0x1a4 */
   uint32_t WRITE_ODT_CTRL;      /* 0x1a8 */
   uint32_t ABI_PAR_CTRL;        /* 0x1ac */
   uint32_t ZQ_CAL;              /* 0x1b0 */
   uint32_t unused6[147];        /* 0x1b4-0x3ff */
} DDRPhyControl;

typedef struct DDRPhyByteLaneControl {
   uint32_t VDL_CTRL_WR[10];     /* 0x00 - 0x27 */
   uint32_t VDL_CTRL_RD[22];     /* 0x28 - 0x7f */
   uint32_t VDL_CLK_CTRL;        /* 0x80 */
   uint32_t VDL_LDE_CTRL;        /* 0x84 */
   uint32_t RD_EN_DLY_CYC;       /* 0x88 */
#define RD_EN_DLY_CYC_CS0_CYCLES_SHIFT      0x0
#define RD_EN_DLY_CYC_CS0_CYCLES_MASK       (0xf<<RD_EN_DLY_CYC_CS0_CYCLES_SHIFT)
   uint32_t WR_CHAN_DLY_CYC;     /* 0x8c */
   uint32_t RD_CTRL;             /* 0x90 */
   uint32_t RD_FIFO_ADDR;        /* 0x94 */
   uint32_t RD_FIFO_DATA;        /* 0x98 */
   uint32_t RD_FIFO_DM_DBI;      /* 0x9c */
   uint32_t RD_FIFO_STATUS;      /* 0xa0 */
   uint32_t RD_FIFO_CLR;         /* 0xa4 */
   uint32_t DYNAMIC_CLK_CTRL;    /* 0xa8 */
   uint32_t IDLE_PAD_CTRL;       /* 0xac */
   uint32_t DRIVE_PAD_CTRL;      /* 0xb0 */
   uint32_t DQSP_DRIVE_PAD_CTRL; /* 0xb4 */
   uint32_t DQSN_DRIVE_PAD_CTRL; /* 0xb8 */
   uint32_t RD_EN_DRIVE_PAD_CTRL;/* 0xbc */
   uint32_t STATIC_PAD_CTRL;     /* 0xc0 */
   uint32_t DQ_RX_TRIM;          /* 0xc4 */
   uint32_t MISC_RX_TRIM;        /* 0xc8 */
   uint32_t DQS_RX_TRIM;         /* 0xcc */
   uint32_t WR_PREAMBLE_MODE;    /* 0xd0 */
   uint32_t ODT_CTRL;            /* 0xd4 */
   uint32_t LDO_CONFIG;          /* 0xd8 */
   uint32_t CLOCK_ENABLE;        /* 0xdc */
   uint32_t CLOCK_IDLE;          /* 0xe0 */
   uint32_t RD_CHAN_FIFO_CLR;    /* 0xe4 */
   uint32_t BL_SPARE_REG;        /* 0xe8 */
   uint32_t unused[69];          /* 0xec-0x1ff */
} DDRPhyByteLaneControl;

typedef struct MEMCControl {
   uint32_t GLB_VERS;               /* 0x000 */
   uint32_t GLB_GCFG;               /* 0x004 */
#define MEMC_GLB_GCFG_DRAM_EN_SHIFT        31
#define MEMC_GLB_GCFG_DRAM_EN_MASK         (0x1<<MEMC_GLB_GCFG_DRAM_EN_SHIFT)
#define MEMC_GLB_GCFG_FORCE_SEQ_SHIFT      30
#define MEMC_GLB_GCFG_FORCE_SEQ_MASK       (0x1<<MEMC_GLB_GCFG_FORCE_SEQ_SHIFT)
#define MEMC_GLB_GCFG_FORCE_SEQ2_SHIFT     29
#define MEMC_GLB_GCFG_FORCE_SEQ2_MASK      (1<<MEMC_GLB_GCFG_FORCE_SEQ2_SHIFT)
#define MEMC_GLB_GCFG_RBF_REORDER_SHIFT    28
#define MEMC_GLB_GCFG_RBF_REORDER_MASK     (1<<MEMC_GLB_GCFG_RBF_REORDER_SHIFT)
#define MEMC_GLB_GCFG_PHY_DFI_MODE_SHIFT   24
#define MEMC_GLB_GCFG_PHY_DFI_MODE_MASK    (0x3<<MEMC_GLB_GCFG_PHY_DFI_MODE_SHIFT)
#define MEMC_GLB_GCFG_ALT_CHN_ARB_SHIFT    21
#define MEMC_GLB_GCFG_ALT_CHN_ARB_MASK     (0x1<<MEMC_GLB_GCFG_ALT_CHN_ARB_SHIFT)
#define MEMC_GLB_GCFG_MaxAge_SHIFT         16
#define MEMC_GLB_GCFG_MaxAge_MASK          (0x1f<<MEMC_GLB_GCFG_MaxAge_SHIFT)
#define MEMC_GLB_GCFG_PHY_DFI_2X           0x0
#define MEMC_GLB_GCFG_PHY_DFI_4X           0x1
#define MEMC_GLB_GCFG_PHY_DFI_8X           0x2
#define MEMC_GLB_GCFG_DFI_EN_SHIFT         10
#define MEMC_GLB_GCFG_DFI_EN_MASK          (0x1<<MEMC_GLB_GCFG_DFI_EN_SHIFT)
#define MEMC_GLB_GCFG_MCLKSRC_SHIFT        9
#define MEMC_GLB_GCFG_MCLKSRC_MASK         (0x1<<MEMC_GLB_GCFG_MCLKSRC_SHIFT)
#define MEMC_GLB_GCFG_MEMINITDONE_SHIFT    8
#define MEMC_GLB_GCFG_MEMINITDONE_MASK     (0x1<<MEMC_GLB_GCFG_MEMINITDONE_SHIFT)
   uint32_t GLB_MRQ_CFG;            /* 0x008 */
   uint32_t unused0;                /* 0x00c */
   uint32_t GLB_FSBL_STATE;         /* 0x010 */
#define MEMC_GLB_FSBL_DRAM_SIZE_SHIFT      0
#define MEMC_GLB_FSBL_DRAM_SIZE_MASK       (0xf << MEMC_GLB_FSBL_DRAM_SIZE_SHIFT)
   uint32_t unused1[3];             /* 0x014 - 0x01f */

   uint32_t SRAM_REMAP_CTRL;        /* 0x020 */
   uint32_t SRAM_REMAP_CTRL_UPPER;  /* 0x024 */
   uint32_t SRAM_REMAP_INIT;        /* 0x028 */
   uint32_t SRAM_REMAP_LOG_INFO_0;  /* 0x02c */
   uint32_t SRAM_REMAP_LOG_INFO_1;  /* 0x030 */
   uint32_t SRAM_REMAP_LOG_INFO_2;  /* 0x034 */
   uint32_t unused2[2];             /* 0x038-0x03f */

   uint32_t RBF_REORDER_0_CFG[16];  /* 0x040 - 0x07f */

   uint32_t INTR2_CPU_STATUS;       /* 0x080 */
   uint32_t INTR2_CPU_SET;          /* 0x084 */
   uint32_t INTR2_CPU_CLEAR;        /* 0x088 */
   uint32_t INTR2_CPU_MASK_STATUS;  /* 0x08c */
   uint32_t INTR2_CPU_MASK_SET;     /* 0x090 */
   uint32_t INTR2_CPU_MASK_CLEAR;   /* 0x094 */
   uint32_t unused3[26];            /* 0x098-0x0ff */

   uint32_t CHN_CFG_CNFG;           /* 0x100 */
#define MEMC_CHN_CFG_CNFG_CS_MODE_SHIFT            16
#define MEMC_CHN_CFG_CNFG_CS_MODE_MASK             (0x3 << MEMC_CHN_CFG_CNFG_CS_MODE_SHIFT)
   uint32_t CHN_CFG_CSST;           /* 0x104 */
   uint32_t unused4[2];             /* 0x108-0x10f */
   uint32_t CHN_CFG_ROW00_0;        /* 0x110 */
   uint32_t CHN_CFG_ROW00_1;        /* 0x114 */
   uint32_t CHN_CFG_ROW01_0;        /* 0x118 */
   uint32_t CHN_CFG_ROW01_1;        /* 0x11c */
   uint32_t CHN_CFG_ROW02_0;        /* 0x120 */
   uint32_t CHN_CFG_COL00_0;        /* 0x124 */
   uint32_t CHN_CFG_COL00_1;        /* 0x128 */
   uint32_t CHN_CFG_COL01_0;        /* 0x12c */
   uint32_t CHN_CFG_COL01_1;        /* 0x130 */
   uint32_t CHN_CFG_BNK10;          /* 0x134 */
   uint32_t CHN_CFG_BG0;            /* 0x138 */
   uint32_t unused5;                /* 0x13c */
   uint32_t CHN_CFG_DRAM_SZ_CHK;    /* 0x140 */
#define MEMC_CHN_CFG_DRAM_SZ_CHK_SHIFT                          4
#define MEMC_CHN_CFG_DRAM_SZ_CHK_MASK                           (0xf<<MEMC_CHN_CFG_DRAM_SZ_CHK_SHIFT)
   uint32_t CHN_CFG_DIAG_SEL;       /* 0x144 */
   uint32_t unused6[46];            /* 0x148-0x1ff */

   uint32_t CHN_TIM_DCMD;           /* 0x200 */
   uint32_t CHN_TIM_DMODE_0;        /* 0x204 */
   uint32_t CHN_TIM_DMODE_2;        /* 0x208 */
   uint32_t CHN_TIM_CLKS;           /* 0x20c */
#define MEMC_CHN_TIM_CLKS_REF_RATE_SHIFT          8
#define MEMC_CHN_TIM_CLKS_REF_DISABLE_SHIFT       31
   uint32_t CHN_TIM_ODT;            /* 0x210 */
   uint32_t CHN_TIM_TIM1_0;         /* 0x214 */
/* MC_CHN_TIM :: TIM1_0 :: TIM1_tWL [31:24] */
#define MC_CHN_TIM_TIM1_0_TIM1_tWL_MASK                            0xff000000
#define MC_CHN_TIM_TIM1_0_TIM1_tWL_ALIGN                           0
#define MC_CHN_TIM_TIM1_0_TIM1_tWL_BITS                            8
#define MC_CHN_TIM_TIM1_0_TIM1_tWL_SHIFT                           24
#define MC_CHN_TIM_TIM1_0_TIM1_tWL_DEFAULT                         0x00000004
/* MC_CHN_TIM :: TIM1_0 :: TIM1_tRP [23:16] */
#define MC_CHN_TIM_TIM1_0_TIM1_tRP_MASK                            0x00ff0000
#define MC_CHN_TIM_TIM1_0_TIM1_tRP_ALIGN                           0
#define MC_CHN_TIM_TIM1_0_TIM1_tRP_BITS                            8
#define MC_CHN_TIM_TIM1_0_TIM1_tRP_SHIFT                           16
#define MC_CHN_TIM_TIM1_0_TIM1_tRP_DEFAULT                         0x00000006
/* MC_CHN_TIM :: TIM1_0 :: TIM1_tCL [15:08] */
#define MC_CHN_TIM_TIM1_0_TIM1_tCL_MASK                            0x0000ff00
#define MC_CHN_TIM_TIM1_0_TIM1_tCL_ALIGN                           0
#define MC_CHN_TIM_TIM1_0_TIM1_tCL_BITS                            8
#define MC_CHN_TIM_TIM1_0_TIM1_tCL_SHIFT                           8
#define MC_CHN_TIM_TIM1_0_TIM1_tCL_DEFAULT                         0x00000005
/* MC_CHN_TIM :: TIM1_0 :: TIM1_tRCD [07:00] */
#define MC_CHN_TIM_TIM1_0_TIM1_tRCD_MASK                           0x000000ff
#define MC_CHN_TIM_TIM1_0_TIM1_tRCD_ALIGN                          0
#define MC_CHN_TIM_TIM1_0_TIM1_tRCD_BITS                           8
#define MC_CHN_TIM_TIM1_0_TIM1_tRCD_SHIFT                          0
#define MC_CHN_TIM_TIM1_0_TIM1_tRCD_DEFAULT                        0x00000006

   uint32_t CHN_TIM_TIM1_1;         /* 0x218 */
/* MC_CHN_TIM :: TIM1_1 :: TIM1_tCCD_L [31:24] */
#define MC_CHN_TIM_TIM1_1_TIM1_tCCD_L_MASK                         0xff000000
#define MC_CHN_TIM_TIM1_1_TIM1_tCCD_L_ALIGN                        0
#define MC_CHN_TIM_TIM1_1_TIM1_tCCD_L_BITS                         8
#define MC_CHN_TIM_TIM1_1_TIM1_tCCD_L_SHIFT                        24
#define MC_CHN_TIM_TIM1_1_TIM1_tCCD_L_DEFAULT                      0x00000002
/* MC_CHN_TIM :: TIM1_1 :: TIM1_tCCD_S [23:16] */
#define MC_CHN_TIM_TIM1_1_TIM1_tCCD_S_MASK                         0x00ff0000
#define MC_CHN_TIM_TIM1_1_TIM1_tCCD_S_ALIGN                        0
#define MC_CHN_TIM_TIM1_1_TIM1_tCCD_S_BITS                         8
#define MC_CHN_TIM_TIM1_1_TIM1_tCCD_S_SHIFT                        16
#define MC_CHN_TIM_TIM1_1_TIM1_tCCD_S_DEFAULT                      0x00000002
/* MC_CHN_TIM :: TIM1_1 :: TIM1_tRRD_L [15:08] */
#define MC_CHN_TIM_TIM1_1_TIM1_tRRD_L_MASK                         0x0000ff00
#define MC_CHN_TIM_TIM1_1_TIM1_tRRD_L_ALIGN                        0
#define MC_CHN_TIM_TIM1_1_TIM1_tRRD_L_BITS                         8
#define MC_CHN_TIM_TIM1_1_TIM1_tRRD_L_SHIFT                        8
#define MC_CHN_TIM_TIM1_1_TIM1_tRRD_L_DEFAULT                      0x00000002
/* MC_CHN_TIM :: TIM1_1 :: TIM1_tRRD_S [07:00] */
#define MC_CHN_TIM_TIM1_1_TIM1_tRRD_S_MASK                         0x000000ff
#define MC_CHN_TIM_TIM1_1_TIM1_tRRD_S_ALIGN                        0
#define MC_CHN_TIM_TIM1_1_TIM1_tRRD_S_BITS                         8
#define MC_CHN_TIM_TIM1_1_TIM1_tRRD_S_SHIFT                        0
#define MC_CHN_TIM_TIM1_1_TIM1_tRRD_S_DEFAULT                      0x00000002

   uint32_t CHN_TIM_TIM1_2;         /* 0x21c */
/* MC_CHN_TIM :: TIM1_2 :: TIM1_tFAW [31:24] */
#define MC_CHN_TIM_TIM1_2_TIM1_tFAW_MASK                           0xff000000
#define MC_CHN_TIM_TIM1_2_TIM1_tFAW_ALIGN                          0
#define MC_CHN_TIM_TIM1_2_TIM1_tFAW_BITS                           8
#define MC_CHN_TIM_TIM1_2_TIM1_tFAW_SHIFT                          24
#define MC_CHN_TIM_TIM1_2_TIM1_tFAW_DEFAULT                        0x00000000
/* MC_CHN_TIM :: TIM1_2 :: reserved0 [23:16] */
#define MC_CHN_TIM_TIM1_2_reserved0_MASK                           0x00ff0000
#define MC_CHN_TIM_TIM1_2_reserved0_ALIGN                          0
#define MC_CHN_TIM_TIM1_2_reserved0_BITS                           8
#define MC_CHN_TIM_TIM1_2_reserved0_SHIFT                          16
/* MC_CHN_TIM :: TIM1_2 :: TIM1_tRTP [15:08] */
#define MC_CHN_TIM_TIM1_2_TIM1_tRTP_MASK                           0x0000ff00
#define MC_CHN_TIM_TIM1_2_TIM1_tRTP_ALIGN                          0
#define MC_CHN_TIM_TIM1_2_TIM1_tRTP_BITS                           8
#define MC_CHN_TIM_TIM1_2_TIM1_tRTP_SHIFT                          8
#define MC_CHN_TIM_TIM1_2_TIM1_tRTP_DEFAULT                        0x00000002
/* MC_CHN_TIM :: TIM1_2 :: TIM1_tRC [07:00] */
#define MC_CHN_TIM_TIM1_2_TIM1_tRC_MASK                            0x000000ff
#define MC_CHN_TIM_TIM1_2_TIM1_tRC_ALIGN                           0
#define MC_CHN_TIM_TIM1_2_TIM1_tRC_BITS                            8
#define MC_CHN_TIM_TIM1_2_TIM1_tRC_SHIFT                           0
#define MC_CHN_TIM_TIM1_2_TIM1_tRC_DEFAULT                         0x00000011

   uint32_t CHN_TIM_TIM1_3;         /* 0x220 */
/* MC_CHN_TIM :: TIM1_3 :: TIM1_tWTR_L [31:24] */
#define MC_CHN_TIM_TIM1_3_TIM1_tWTR_L_MASK                         0xff000000
#define MC_CHN_TIM_TIM1_3_TIM1_tWTR_L_ALIGN                        0
#define MC_CHN_TIM_TIM1_3_TIM1_tWTR_L_BITS                         8
#define MC_CHN_TIM_TIM1_3_TIM1_tWTR_L_SHIFT                        24
#define MC_CHN_TIM_TIM1_3_TIM1_tWTR_L_DEFAULT                      0x00000002
/* MC_CHN_TIM :: TIM1_3 :: TIM1_tWTR_S [23:16] */
#define MC_CHN_TIM_TIM1_3_TIM1_tWTR_S_MASK                         0x00ff0000
#define MC_CHN_TIM_TIM1_3_TIM1_tWTR_S_ALIGN                        0
#define MC_CHN_TIM_TIM1_3_TIM1_tWTR_S_BITS                         8
#define MC_CHN_TIM_TIM1_3_TIM1_tWTR_S_SHIFT                        16
#define MC_CHN_TIM_TIM1_3_TIM1_tWTR_S_DEFAULT                      0x00000002
/* MC_CHN_TIM :: TIM1_3 :: reserved_for_padding0 [15:12] */
#define MC_CHN_TIM_TIM1_3_reserved_for_padding0_MASK               0x0000f000
#define MC_CHN_TIM_TIM1_3_reserved_for_padding0_ALIGN              0
#define MC_CHN_TIM_TIM1_3_reserved_for_padding0_BITS               4
#define MC_CHN_TIM_TIM1_3_reserved_for_padding0_SHIFT              12
/* MC_CHN_TIM :: TIM1_3 :: TIM1_tWR_L [11:08] */
#define MC_CHN_TIM_TIM1_3_TIM1_tWR_L_MASK                          0x00000f00
#define MC_CHN_TIM_TIM1_3_TIM1_tWR_L_ALIGN                         0
#define MC_CHN_TIM_TIM1_3_TIM1_tWR_L_BITS                          4
#define MC_CHN_TIM_TIM1_3_TIM1_tWR_L_SHIFT                         8
#define MC_CHN_TIM_TIM1_3_TIM1_tWR_L_DEFAULT                       0x00000004
/* MC_CHN_TIM :: TIM1_3 :: TIM1_tWR_S [07:00] */
#define MC_CHN_TIM_TIM1_3_TIM1_tWR_S_MASK                          0x000000ff
#define MC_CHN_TIM_TIM1_3_TIM1_tWR_S_ALIGN                         0
#define MC_CHN_TIM_TIM1_3_TIM1_tWR_S_BITS                          8
#define MC_CHN_TIM_TIM1_3_TIM1_tWR_S_SHIFT                         0
#define MC_CHN_TIM_TIM1_3_TIM1_tWR_S_DEFAULT                       0x00000004

   uint32_t CHN_TIM_TIM2;          /* 0x224 */
/* MC_CHN_TIM :: TIM2 :: TIM2_tR2R [31:30] */
#define MC_CHN_TIM_TIM2_TIM2_tR2R_MASK                             0xc0000000
#define MC_CHN_TIM_TIM2_TIM2_tR2R_ALIGN                            0
#define MC_CHN_TIM_TIM2_TIM2_tR2R_BITS                             2
#define MC_CHN_TIM_TIM2_TIM2_tR2R_SHIFT                            30
#define MC_CHN_TIM_TIM2_TIM2_tR2R_DEFAULT                          0x00000000
/* MC_CHN_TIM :: TIM2 :: TIM2_tR2W [29:27] */
#define MC_CHN_TIM_TIM2_TIM2_tR2W_MASK                             0x38000000
#define MC_CHN_TIM_TIM2_TIM2_tR2W_ALIGN                            0
#define MC_CHN_TIM_TIM2_TIM2_tR2W_BITS                             3
#define MC_CHN_TIM_TIM2_TIM2_tR2W_SHIFT                            27
#define MC_CHN_TIM_TIM2_TIM2_tR2W_DEFAULT                          0x00000001
/* MC_CHN_TIM :: TIM2 :: TIM2_tW2R [26:24] */
#define MC_CHN_TIM_TIM2_TIM2_tW2R_MASK                             0x07000000
#define MC_CHN_TIM_TIM2_TIM2_tW2R_ALIGN                            0
#define MC_CHN_TIM_TIM2_TIM2_tW2R_BITS                             3
#define MC_CHN_TIM_TIM2_TIM2_tW2R_SHIFT                            24
#define MC_CHN_TIM_TIM2_TIM2_tW2R_DEFAULT                          0x00000001
/* MC_CHN_TIM :: TIM2 :: reserved_for_padding0 [23:22] */
#define MC_CHN_TIM_TIM2_reserved_for_padding0_MASK                 0x00c00000
#define MC_CHN_TIM_TIM2_reserved_for_padding0_ALIGN                0
#define MC_CHN_TIM_TIM2_reserved_for_padding0_BITS                 2
#define MC_CHN_TIM_TIM2_reserved_for_padding0_SHIFT                22
/* MC_CHN_TIM :: TIM2 :: TIM2_tW2W [21:18] */
#define MC_CHN_TIM_TIM2_TIM2_tW2W_MASK                             0x003c0000
#define MC_CHN_TIM_TIM2_TIM2_tW2W_ALIGN                            0
#define MC_CHN_TIM_TIM2_TIM2_tW2W_BITS                             4
#define MC_CHN_TIM_TIM2_TIM2_tW2W_SHIFT                            18
#define MC_CHN_TIM_TIM2_TIM2_tW2W_DEFAULT                          0x00000000
/* MC_CHN_TIM :: TIM2 :: reserved1 [17:16] */
#define MC_CHN_TIM_TIM2_reserved1_MASK                             0x00030000
#define MC_CHN_TIM_TIM2_reserved1_ALIGN                            0
#define MC_CHN_TIM_TIM2_reserved1_BITS                             2
#define MC_CHN_TIM_TIM2_reserved1_SHIFT                            16
/* MC_CHN_TIM :: TIM2 :: TIM2_tAL [15:12] */
#define MC_CHN_TIM_TIM2_TIM2_tAL_MASK                              0x0000f000
#define MC_CHN_TIM_TIM2_TIM2_tAL_ALIGN                             0
#define MC_CHN_TIM_TIM2_TIM2_tAL_BITS                              4
#define MC_CHN_TIM_TIM2_TIM2_tAL_SHIFT                             12
#define MC_CHN_TIM_TIM2_TIM2_tAL_DEFAULT                           0x00000000
/* MC_CHN_TIM :: TIM2 :: TIM2_tRFC [11:00] */
#define MC_CHN_TIM_TIM2_TIM2_tRFC_MASK                             0x00000fff
#define MC_CHN_TIM_TIM2_TIM2_tRFC_ALIGN                            0
#define MC_CHN_TIM_TIM2_TIM2_tRFC_BITS                             12
#define MC_CHN_TIM_TIM2_TIM2_tRFC_SHIFT                            0
#define MC_CHN_TIM_TIM2_TIM2_tRFC_DEFAULT                          0x00000014

   uint32_t unused7[2];            /* 0x228 - 0x22f */
   uint32_t CHN_TIM_PHY_ST;         /* 0x230 */
#define MEMC_CHN_TIM_PHY_ST_PHY_ST_POWER_UP       0x1
#define MEMC_CHN_TIM_PHY_ST_PHY_ST_HW_RESET       0x2
#define MEMC_CHN_TIM_PHY_ST_PHY_ST_SW_RESET       0x4
#define MEMC_CHN_TIM_PHY_ST_PHY_ST_READY          0x10
   uint32_t CHN_TIM_DRAM_CFG;       /* 0x234 */
#define MEMC_CHN_TIM_DRAM_CFG_HDP_SHIFT           12
#define MEMC_CHN_TIM_DRAM_CFG_HDP_MASK            (0x1<<MEMC_CHN_TIM_DRAM_CFG_HDP_SHIFT)
#define MEMC_CHN_TIM_DRAM_CFG_2TADDRCMD_SHIFT     10
#define MEMC_CHN_TIM_DRAM_CFG_2TADDRCMD_MASK      (0x1<<MEMC_CHN_TIM_DRAM_CFG_2TADDRCMD_SHIFT)
#define MEMC_CHN_TIM_DRAM_CFG_DRAMTYPE_SHIFT      0
#define MEMC_CHN_TIM_DRAM_CFG_DRAMTYPE_MASK       (0xf<<MEMC_CHN_TIM_DRAM_CFG_DRAMTYPE_SHIFT)
#define MC_DRAM_CFG_DDR3                          0x1
#define MC_DRAM_CFG_DDR4                          0x2
   uint32_t CHN_TIM_STAT;           /* 0x238 */
   uint32_t CHN_TIM_PERF;           /* 0x23c */

   uint32_t CHN_TIM_PKM_CFG;        /* 0x240 */
#define MC_CHN_TIM_PKM_CFG_ENABLE_MASK                             0x00000001
#define MC_CHN_TIM_PKM_CFG_ENABLE_ALIGN                            0
#define MC_CHN_TIM_PKM_CFG_ENABLE_BITS                             1
#define MC_CHN_TIM_PKM_CFG_ENABLE_SHIFT                            0
#define MC_CHN_TIM_PKM_CFG_ENABLE_DEFAULT                          0x00000000
   uint32_t CHN_TIM_AB4_CFG;        /* 0x244 */
#define MC_CHN_TIM_AB4_CFG_Disable_MASK                            0x80000000
#define MC_CHN_TIM_AB4_CFG_Disable_ALIGN                           0
#define MC_CHN_TIM_AB4_CFG_Disable_BITS                            1
#define MC_CHN_TIM_AB4_CFG_Disable_SHIFT                           31
#define MC_CHN_TIM_AB4_CFG_Disable_DEFAULT                         0x00000000

#define MC_CHN_TIM_AB4_CFG_BURST_MASK                              0x40000000
#define MC_CHN_TIM_AB4_CFG_BURST_ALIGN                             0
#define MC_CHN_TIM_AB4_CFG_BURST_BITS                              1
#define MC_CHN_TIM_AB4_CFG_BURST_SHIFT                             30
#define MC_CHN_TIM_AB4_CFG_BURST_DEFAULT                           0x00000001

#define MC_CHN_TIM_AB4_CFG_RdWrWeight1_MASK                        0x00003f00
#define MC_CHN_TIM_AB4_CFG_RdWrWeight1_ALIGN                       0
#define MC_CHN_TIM_AB4_CFG_RdWrWeight1_BITS                        6
#define MC_CHN_TIM_AB4_CFG_RdWrWeight1_SHIFT                       8
#define MC_CHN_TIM_AB4_CFG_RdWrWeight1_DEFAULT                     0x00000002

#define MC_CHN_TIM_AB4_CFG_RdWrWeight0_MASK                        0x0000003f
#define MC_CHN_TIM_AB4_CFG_RdWrWeight0_ALIGN                       0
#define MC_CHN_TIM_AB4_CFG_RdWrWeight0_BITS                        6
#define MC_CHN_TIM_AB4_CFG_RdWrWeight0_SHIFT                       0
#define MC_CHN_TIM_AB4_CFG_RdWrWeight0_DEFAULT                     0x00000002

   uint32_t unused8[2];             /* 0x248 */
   uint32_t CHN_TIM_AB4_ACT_PRI;    /* 0x250 */
   uint32_t CHN_TIM_AB4_PRI_OPEN;   /* 0x254 */
   uint32_t CHN_TIM_AB4_PRI_CLOSED; /* 0x258 */
   uint32_t CHN_TIM_AUTO_SELF_REF;  /* 0x25c */
   uint32_t CHN_TIM_AUTO_ZQCS;      /* 0x260 */
   uint32_t CHN_TIM_TIM3;           /* 0x264 */
#define MC_CHN_TIM_TIM3_TIM3_TZQCL_MASK                            0x0fff0000
#define MC_CHN_TIM_TIM3_TIM3_TZQCL_ALIGN                           0
#define MC_CHN_TIM_TIM3_TIM3_TZQCL_BITS                            12
#define MC_CHN_TIM_TIM3_TIM3_TZQCL_SHIFT                           16
#define MC_CHN_TIM_TIM3_TIM3_TZQCL_DEFAULT                         0x00000100
#define MC_CHN_TIM_TIM3_TIM3_TZQCS_MASK                            0x000000ff
#define MC_CHN_TIM_TIM3_TIM3_TZQCS_ALIGN                           0
#define MC_CHN_TIM_TIM3_TIM3_TZQCS_BITS                            8
#define MC_CHN_TIM_TIM3_TIM3_TZQCS_SHIFT                           0
#define MC_CHN_TIM_TIM3_TIM3_TZQCS_DEFAULT                         0x00000040
   uint32_t unused9[38];            /* 0x268-0x2ff */

   uint32_t ARB_CFG;                /* 0x300 */
#define MC_ARB_CFG_ARB_MODE_MASK                                   0x00000001
#define MC_ARB_CFG_ARB_MODE_SHIFT                                  0
#define MC_ARB_CFG_CPU_PER_SRC_RL_EN_MASK                          0x00000004
#define MC_ARB_CFG_CPU_PER_SRC_RL_EN_SHIFT                         2
#define MC_ARB_CFG_CPQ_PER_SRC_RL_EN_MASK                          0x00000008
#define MC_ARB_CFG_CPQ_PER_SRC_RL_EN_SHIFT                         3
#define MC_ARB_CFG_RATE_FAILED_RR_DIS_MASK                         0xffff0000
#define MC_ARB_CFG_RATE_FAILED_RR_DIS_SHIFT                        16
   uint32_t unused10;               /* 0x304 */
   uint32_t ARB_SP_PR[2];           /* 0x308 */
   uint32_t ARB_BKT_CFG_CPU_WR[4];  /* 0x310 */
   uint32_t ARB_BKT_CFG_CPU_RD[4];  /* 0x320 */
   uint32_t ARB_BKT_CFG_CPQ_RD[4];  /* 0x330 */
   uint32_t ARB_BKT_CFG_CPQ_WR[4];  /* 0x340 */
   uint32_t ARB_BKT_CFG_EDIS[4];    /* 0x350 */
   uint32_t unused11[32];           /* 0x360 */
   uint32_t ARB_QUEUE_PRI[3];       /* 0x3e0 */
   uint32_t ARM_WRR_QUEUE_PRI;      /* 0x3ec */
   uint32_t ARB_RATE_OK_WEIGHT[2];  /* 0x3f0 */
   uint32_t ARB_RATE_FAIL_WEIGHT[2]; /* 0x3f4 */

   uint32_t unused11_1[48];         /* 0x400 */
   AXIInterface AXIWIF;           /* 0x4c0 */
   uint32_t unused13[192];          /* 0x500-0x7ff */

   EDISEngine EDIS_0;             /* 0x800 */
   uint32_t unused14[64];           /* 0x900 */

   uint32_t STATS_CTRL;             /* 0xa00 */
   uint32_t STATS_TIMER_CFG;        /* 0xa04 */
   uint32_t STATS_TIMER_COUNT;      /* 0xa08 */
   uint32_t STATS_TOTAL_SLICE;      /* 0xa0c */
   uint32_t STATS_TOTAL_PACKET;     /* 0xa10 */
   uint32_t STATS_TOTAL_READ_SLICE; /* 0xa14 */
   uint32_t STATS_TOTAL_READ_PACKET;/* 0xa18 */
   uint32_t STATS_TOTAL_LATENCY;    /* 0xa1c */
   uint32_t STAT_MAX_LATENCY;       /* 0xa20 */
   uint32_t STATS_SLICE_REORDER;    /* 0xa24 */
   uint32_t STATS_TOTAL_DDR_CMD;    /* 0xa28 */
   uint32_t STATS_TOTAL_DDR_ACT;    /* 0xa2c */
   uint32_t STATS_TOTAL_DDR_RDWR;   /* 0xa30 */
   uint32_t STATS_TOTAL_DDR_WRRD;   /* 0xa34 */
   uint32_t STATS_TOTAL_MRQ_CHN_FULL; /* 0xa38 */
   uint32_t STATS_TOTAL_SELF_REF;   /* 0xa3c */
   uint32_t STATS_ARB_GRANT_MATCH0; /* 0xa40 */
   uint32_t STATS_TOTAL_QUEUE_FULL; /* 0xa44 */
   uint32_t STATS_TOTAL_ARB_GRANT;  /* 0xa48 */
   uint32_t STATS_TOTAL_RATE_OK_GRANT; /* 0xa4c*/
   uint32_t STATS_FILTER_CFG_0;     /* 0xa50 */
#define MEMC_STATS_FILTER_CFG_MATCH_SRC_SHIFT  25
#define MEMC_STATS_FILTER_CFG_MATCH_SRC_MASK   (0x1<<MEMC_STATS_FILTER_CFG_MATCH_SRC_SHIFT)
#define MEMC_STATS_FILTER_CFG_MATCH_SRC_EN     0x02000000
#define MEMC_STATS_FILTER_CFG_SRC_ID_SHIFT     16
#define MEMC_STATS_FILTER_CFG_SRC_ID_MASK      (0x1FF<<MEMC_STATS_FILTER_CFG_SRC_ID_SHIFT)
#define MEMC_STATS_FILTER_CFG_MATCH_INTF_SHIFT 0
#define MEMC_STATS_FILTER_CFG_MATCH_INTF_MASK  0x0000FFFF
#define MEMC_STATS_FILTER_CFG_INTF_AXI         0x00000001
#define MEMC_STATS_FILTER_CFG_INTF_EDIS0       0x00000002
#define MEMC_STATS_FILTER_CFG_INTF_UNUSED      0x00000008
#define MEMC_STATS_FILTER_CFG_INTF_UBUS        MEMC_STATS_FILTER_CFG_INTF_UNUSED
#define MEMC_STATS_FILTER_CFG_INTF_UBUS0       MEMC_STATS_FILTER_CFG_INTF_UNUSED
#define MEMC_STATS_FILTER_CFG_INTF_UBUS1       MEMC_STATS_FILTER_CFG_INTF_UNUSED
#define MEMC_STATS_FILTER_CFG_INTF_MCP         MEMC_STATS_FILTER_CFG_INTF_UNUSED
#define MEMC_STATS_FILTER_CFG_INTF_EDIS1       MEMC_STATS_FILTER_CFG_INTF_UNUSED
   uint32_t STATS_PROG0_SLICE;      /* 0xa54 */
   uint32_t STATS_PROG0_PACKET;     /* 0xa58 */
   uint32_t STATS_PROG0_READ_SLICE; /* 0xa5c */
   uint32_t STATS_PROG0_READ_PACKET; /* 0xa60 */
   uint32_t STATS_PROG0_LATENCY;    /* 0xa64 */
   uint32_t STATS_PROG0_MAX_LETANCY; /* 0xa68 */
   uint32_t unused15_0;              /* 0xa6c */
   uint32_t STATS_FILTER_CFG_1;     /* 0xa70 */
   uint32_t STATS_PROG1_SLICE;      /* 0xa74 */
   uint32_t STATS_PROG1_PACKET;     /* 0xa78 */
   uint32_t STATS_PROG1_READ_SLICE; /* 0xa7c */
   uint32_t STATS_PROG1_READ_PACKET;/* 0xa80 */
   uint32_t STATS_PROG1_LATENCY;    /* 0xa84 */
   uint32_t unused15_1[2];          /* 0xa88 - 0xa8c */
   uint32_t STATS_FILTER_CFG_2;     /* 0xa90 */
   uint32_t STATS_PROG2_SLICE;      /* 0xa94 */
   uint32_t STATS_PROG2_PACKET;     /* 0xa98 */
   uint32_t STATS_PROG2_READ_SLICE; /* 0xa9c */
   uint32_t STATS_PROG2_READ_PACKET;/* 0xaa0 */
   uint32_t STATS_PROG2_LATENCY;    /* 0xaa4 */
   uint32_t unused15_2[2];          /* 0xaa8 - 0xaac */
   uint32_t STATS_FILTER_CFG_3;     /* 0xab0 */
   uint32_t STATS_PROG3_SLICE;      /* 0xab4 */
   uint32_t STATS_PROG3_PACKET;     /* 0xab8 */
   uint32_t STATS_PROG3_READ_SLICE; /* 0xabc */
   uint32_t STATS_PROG3_READ_PACKET;/* 0xac0 */
   uint32_t STATS_PROG3_LATENCY;    /* 0xac4 */
   uint32_t unused15_3[2];            /* 0xac8 - 0xacc */
   uint32_t STATS_AXI_BLOCK_CPU_RD; /* 0xad0 */
   uint32_t STATS_AXI_BLOCK_CPQ_RD; /* 0xad4 */
   uint32_t unused15_4[10];           /* 0xad8 - 0xaff */

   uint32_t CAP_CAPTURE_CFG;        /* 0xb00 */
   uint32_t CAP_TRIGGER_ADDR;       /* 0xb04 */
   uint32_t CAP_READ_CTRL;          /* 0xb08 */
   uint32_t unused16;               /* 0xb0c */
   uint32_t CAP_CAPTURE_MATCH0;     /* 0xb10 */
   uint32_t CAP_CAPTURE_MATCH1;     /* 0xb14 */
   uint32_t CAP_CAPTURE_MATCH2;     /* 0xb18 */
   uint32_t unused17;               /* 0xb1c */
   uint32_t CAP_CAPTURE_MASK0;      /* 0xb20 */
   uint32_t CAP_CAPTURE_MASK1;      /* 0xb24 */
   uint32_t CAP_CAPTURE_MASK2;      /* 0xb28 */
   uint32_t unused18;               /* 0xb2c */
   uint32_t CAP_TRIGGER_MATCH0;     /* 0xb30 */
   uint32_t CAP_TRIGGER_MATCH1;     /* 0xb34 */
   uint32_t CAP_TRIGGER_MATCH2;     /* 0xb38 */
   uint32_t unused19;               /* 0xb3c */
   uint32_t CAP_TRIGGER_MASK0;      /* 0xb40 */
   uint32_t CAP_TRIGGER_MASK1;      /* 0xb44 */
   uint32_t CAP_TRIGGER_MASK2;      /* 0xb48 */
   uint32_t unused20;               /* 0xb4c */
   uint32_t CAP_READ_DATA[8];       /* 0xb50-0xb6f */
   uint32_t unused21[164];          /* 0xb70-0xdff */

   uint32_t SEC_INTR2_CPU_STATUS;   /* 0xe00 */
   uint32_t SEC_INTR2_CPU_SET;      /* 0xe04 */
   uint32_t SEC_INTR2_CPU_CLEAR;    /* 0xe08 */
   uint32_t SEC_INTR2_CPU_MASK_STATUS;  /* 0xe0c */
   uint32_t SEC_INTR2_CPU_MASK_SET;  /* 0xe10 */
   uint32_t SEC_INTR2_CPU_MASK_CLEAR;   /* 0xe14 */
   uint32_t unused22[58];              /* 0xe18-0xeff */

   uint32_t UBUSIF0_PERMCTL;          /* 0xf00 */
   uint32_t UBUSIF1_PERMCTL;          /* 0xf04 */
   uint32_t SRAM_REMAP_PERMCTL;       /* 0xf08 */
   uint32_t AXIWIF_PERMCTL;           /* 0xf0c */
   uint32_t CHNCFG_PERMCTL;           /* 0xf10 */
   uint32_t MCCAP_PERMCTL;            /* 0xf14 */
   uint32_t SCRAM_PERMCTL;            /* 0xf18 */
   uint32_t RNG_PERMCTL;              /* 0xf1c */
   uint32_t RNGCHK_PERMCTL;           /* 0xf20 */
   uint32_t GLB_PERMCTL;              /* 0xf24 */
   uint32_t unused23[566];            /* 0xf28-0x17ff */

   SecureRangeCheckers SEC_RANGE_CHK; /* 0x1800-0x18ff */
   uint32_t unused24[31168];            /* 0x1900-0x200000 */

   DDRPhyControl PhyControl;                    /* 0x20000 */
   DDRPhyByteLaneControl PhyByteLaneControl[4]; /* 0x20400 - 0x20bff */
} MEMCControl;

#define MEMC ((volatile MEMCControl * const) MEMC_BASE)

#endif /* _6855_DDR_H */

