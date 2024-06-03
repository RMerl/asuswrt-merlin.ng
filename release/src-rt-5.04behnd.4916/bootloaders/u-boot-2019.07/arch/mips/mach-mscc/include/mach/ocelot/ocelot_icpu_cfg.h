/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#ifndef _MSCC_OCELOT_ICPU_CFG_H_
#define _MSCC_OCELOT_ICPU_CFG_H_

#define ICPU_GPR(x) (0x4 * (x))
#define ICPU_GPR_RSZ                                      0x4

#define ICPU_RESET                                        0x20

#define ICPU_RESET_CORE_RST_CPU_ONLY                      BIT(3)
#define ICPU_RESET_CORE_RST_PROTECT                       BIT(2)
#define ICPU_RESET_CORE_RST_FORCE                         BIT(1)
#define ICPU_RESET_MEM_RST_FORCE                          BIT(0)

#define ICPU_GENERAL_CTRL                                 0x24

#define ICPU_GENERAL_CTRL_CPU_BUSIF_SLEEP_DIS             BIT(14)
#define ICPU_GENERAL_CTRL_CPU_BUSIF_WERR_ENA              BIT(13)
#define ICPU_GENERAL_CTRL_CPU_8051_IROM_ENA               BIT(12)
#define ICPU_GENERAL_CTRL_CPU_MIPS_DIS                    BIT(11)
#define ICPU_GENERAL_CTRL_IF_MIIM_SLV_ADDR_SEL            BIT(10)
#define ICPU_GENERAL_CTRL_IF_MIIM_SLV_ENA                 BIT(9)
#define ICPU_GENERAL_CTRL_IF_PI_SLV_DONEPOL               BIT(8)
#define ICPU_GENERAL_CTRL_IF_PI_MST_ENA                   BIT(7)
#define ICPU_GENERAL_CTRL_IF_PI_SLV_ENA                   BIT(6)
#define ICPU_GENERAL_CTRL_IF_SI_OWNER(x)                  (((x) << 4) & GENMASK(5, 4))
#define ICPU_GENERAL_CTRL_IF_SI_OWNER_M                   GENMASK(5, 4)
#define ICPU_GENERAL_CTRL_IF_SI_OWNER_X(x)                (((x) & GENMASK(5, 4)) >> 4)
#define ICPU_GENERAL_CTRL_SSI_MST_CONTENTION              BIT(3)
#define ICPU_GENERAL_CTRL_CPU_BE_ENA                      BIT(2)
#define ICPU_GENERAL_CTRL_CPU_DIS                         BIT(1)
#define ICPU_GENERAL_CTRL_BOOT_MODE_ENA                   BIT(0)
#define ICPU_SPI_MST_CFG                                  0x3c

#define ICPU_SPI_MST_CFG_A32B_ENA                         BIT(11)
#define ICPU_SPI_MST_CFG_FAST_READ_ENA                    BIT(10)
#define ICPU_SPI_MST_CFG_CS_DESELECT_TIME(x)              (((x) << 5) & GENMASK(9, 5))
#define ICPU_SPI_MST_CFG_CS_DESELECT_TIME_M               GENMASK(9, 5)
#define ICPU_SPI_MST_CFG_CS_DESELECT_TIME_X(x)            (((x) & GENMASK(9, 5)) >> 5)
#define ICPU_SPI_MST_CFG_CLK_DIV(x)                       ((x) & GENMASK(4, 0))
#define ICPU_SPI_MST_CFG_CLK_DIV_M                        GENMASK(4, 0)

#define ICPU_SW_MODE                                      0x50

#define ICPU_SW_MODE_SW_PIN_CTRL_MODE                     BIT(13)
#define ICPU_SW_MODE_SW_SPI_SCK                           BIT(12)
#define ICPU_SW_MODE_SW_SPI_SCK_OE                        BIT(11)
#define ICPU_SW_MODE_SW_SPI_SDO                           BIT(10)
#define ICPU_SW_MODE_SW_SPI_SDO_OE                        BIT(9)
#define ICPU_SW_MODE_SW_SPI_CS(x)                         (((x) << 5) & GENMASK(8, 5))
#define ICPU_SW_MODE_SW_SPI_CS_M                          GENMASK(8, 5)
#define ICPU_SW_MODE_SW_SPI_CS_X(x)                       (((x) & GENMASK(8, 5)) >> 5)
#define ICPU_SW_MODE_SW_SPI_CS_OE(x)                      (((x) << 1) & GENMASK(4, 1))
#define ICPU_SW_MODE_SW_SPI_CS_OE_M                       GENMASK(4, 1)
#define ICPU_SW_MODE_SW_SPI_CS_OE_X(x)                    (((x) & GENMASK(4, 1)) >> 1)
#define ICPU_SW_MODE_SW_SPI_SDI                           BIT(0)

#define ICPU_INTR_ENA					  0x88

#define ICPU_DST_INTR_MAP(x)  (0x98 + 0x4 * (x))
#define ICPU_DST_INTR_MAP_RSZ                             0x4

#define ICPU_DST_INTR_IDENT                               0xa8
#define ICPU_DST_INTR_IDENT_RSZ                           0x4

#define ICPU_TIMER_TICK_DIV                               0xe8
#define ICPU_TIMER_VALUE(x) (0xec + 0x4 * (x))

#define ICPU_TIMER_CTRL(x) (0x104 + 0x4 * (x))
#define ICPU_TIMER_CTRL_MAX_FREQ_ENA			  BIT(3)
#define ICPU_TIMER_CTRL_ONE_SHOT_ENA			  BIT(2)
#define ICPU_TIMER_CTRL_TIMER_ENA			  BIT(1)
#define ICPU_TIMER_CTRL_FORCE_RELOAD			  BIT(0)

#define ICPU_MEMCTRL_CTRL                                 0x110
#define ICPU_MEMCTRL_CTRL_PWR_DOWN                        BIT(3)
#define ICPU_MEMCTRL_CTRL_MDSET                           BIT(2)
#define ICPU_MEMCTRL_CTRL_STALL_REF_ENA                   BIT(1)
#define ICPU_MEMCTRL_CTRL_INITIALIZE                      BIT(0)

#define ICPU_MEMCTRL_CFG                                  0x114

#define ICPU_MEMCTRL_CFG_DDR_512MBYTE_PLUS                BIT(16)
#define ICPU_MEMCTRL_CFG_DDR_ECC_ERR_ENA                  BIT(15)
#define ICPU_MEMCTRL_CFG_DDR_ECC_COR_ENA                  BIT(14)
#define ICPU_MEMCTRL_CFG_DDR_ECC_ENA                      BIT(13)
#define ICPU_MEMCTRL_CFG_DDR_WIDTH                        BIT(12)
#define ICPU_MEMCTRL_CFG_DDR_MODE                         BIT(11)
#define ICPU_MEMCTRL_CFG_BURST_SIZE                       BIT(10)
#define ICPU_MEMCTRL_CFG_BURST_LEN                        BIT(9)
#define ICPU_MEMCTRL_CFG_BANK_CNT                         BIT(8)
#define ICPU_MEMCTRL_CFG_MSB_ROW_ADDR(x)                  (((x) << 4) & GENMASK(7, 4))
#define ICPU_MEMCTRL_CFG_MSB_ROW_ADDR_M                   GENMASK(7, 4)
#define ICPU_MEMCTRL_CFG_MSB_ROW_ADDR_X(x)                (((x) & GENMASK(7, 4)) >> 4)
#define ICPU_MEMCTRL_CFG_MSB_COL_ADDR(x)                  ((x) & GENMASK(3, 0))
#define ICPU_MEMCTRL_CFG_MSB_COL_ADDR_M                   GENMASK(3, 0)

#define ICPU_MEMCTRL_STAT                                 0x118

#define ICPU_MEMCTRL_STAT_RDATA_MASKED                    BIT(5)
#define ICPU_MEMCTRL_STAT_RDATA_DUMMY                     BIT(4)
#define ICPU_MEMCTRL_STAT_RDATA_ECC_ERR                   BIT(3)
#define ICPU_MEMCTRL_STAT_RDATA_ECC_COR                   BIT(2)
#define ICPU_MEMCTRL_STAT_PWR_DOWN_ACK                    BIT(1)
#define ICPU_MEMCTRL_STAT_INIT_DONE                       BIT(0)

#define ICPU_MEMCTRL_REF_PERIOD                           0x11c

#define ICPU_MEMCTRL_REF_PERIOD_MAX_PEND_REF(x)           (((x) << 16) & GENMASK(19, 16))
#define ICPU_MEMCTRL_REF_PERIOD_MAX_PEND_REF_M            GENMASK(19, 16)
#define ICPU_MEMCTRL_REF_PERIOD_MAX_PEND_REF_X(x)         (((x) & GENMASK(19, 16)) >> 16)
#define ICPU_MEMCTRL_REF_PERIOD_REF_PERIOD(x)             ((x) & GENMASK(15, 0))
#define ICPU_MEMCTRL_REF_PERIOD_REF_PERIOD_M              GENMASK(15, 0)

#define ICPU_MEMCTRL_TIMING0                              0x124

#define ICPU_MEMCTRL_TIMING0_RD_TO_WR_DLY(x)              (((x) << 28) & GENMASK(31, 28))
#define ICPU_MEMCTRL_TIMING0_RD_TO_WR_DLY_M               GENMASK(31, 28)
#define ICPU_MEMCTRL_TIMING0_RD_TO_WR_DLY_X(x)            (((x) & GENMASK(31, 28)) >> 28)
#define ICPU_MEMCTRL_TIMING0_WR_CS_CHANGE_DLY(x)          (((x) << 24) & GENMASK(27, 24))
#define ICPU_MEMCTRL_TIMING0_WR_CS_CHANGE_DLY_M           GENMASK(27, 24)
#define ICPU_MEMCTRL_TIMING0_WR_CS_CHANGE_DLY_X(x)        (((x) & GENMASK(27, 24)) >> 24)
#define ICPU_MEMCTRL_TIMING0_RD_CS_CHANGE_DLY(x)          (((x) << 20) & GENMASK(23, 20))
#define ICPU_MEMCTRL_TIMING0_RD_CS_CHANGE_DLY_M           GENMASK(23, 20)
#define ICPU_MEMCTRL_TIMING0_RD_CS_CHANGE_DLY_X(x)        (((x) & GENMASK(23, 20)) >> 20)
#define ICPU_MEMCTRL_TIMING0_RAS_TO_PRECH_DLY(x)          (((x) << 16) & GENMASK(19, 16))
#define ICPU_MEMCTRL_TIMING0_RAS_TO_PRECH_DLY_M           GENMASK(19, 16)
#define ICPU_MEMCTRL_TIMING0_RAS_TO_PRECH_DLY_X(x)        (((x) & GENMASK(19, 16)) >> 16)
#define ICPU_MEMCTRL_TIMING0_WR_TO_PRECH_DLY(x)           (((x) << 12) & GENMASK(15, 12))
#define ICPU_MEMCTRL_TIMING0_WR_TO_PRECH_DLY_M            GENMASK(15, 12)
#define ICPU_MEMCTRL_TIMING0_WR_TO_PRECH_DLY_X(x)         (((x) & GENMASK(15, 12)) >> 12)
#define ICPU_MEMCTRL_TIMING0_RD_TO_PRECH_DLY(x)           (((x) << 8) & GENMASK(11, 8))
#define ICPU_MEMCTRL_TIMING0_RD_TO_PRECH_DLY_M            GENMASK(11, 8)
#define ICPU_MEMCTRL_TIMING0_RD_TO_PRECH_DLY_X(x)         (((x) & GENMASK(11, 8)) >> 8)
#define ICPU_MEMCTRL_TIMING0_WR_DATA_XFR_DLY(x)           (((x) << 4) & GENMASK(7, 4))
#define ICPU_MEMCTRL_TIMING0_WR_DATA_XFR_DLY_M            GENMASK(7, 4)
#define ICPU_MEMCTRL_TIMING0_WR_DATA_XFR_DLY_X(x)         (((x) & GENMASK(7, 4)) >> 4)
#define ICPU_MEMCTRL_TIMING0_RD_DATA_XFR_DLY(x)           ((x) & GENMASK(3, 0))
#define ICPU_MEMCTRL_TIMING0_RD_DATA_XFR_DLY_M            GENMASK(3, 0)

#define ICPU_MEMCTRL_TIMING1                              0x128

#define ICPU_MEMCTRL_TIMING1_RAS_TO_RAS_SAME_BANK_DLY(x)  (((x) << 24) & GENMASK(31, 24))
#define ICPU_MEMCTRL_TIMING1_RAS_TO_RAS_SAME_BANK_DLY_M   GENMASK(31, 24)
#define ICPU_MEMCTRL_TIMING1_RAS_TO_RAS_SAME_BANK_DLY_X(x) (((x) & GENMASK(31, 24)) >> 24)
#define ICPU_MEMCTRL_TIMING1_BANK8_FAW_DLY(x)             (((x) << 16) & GENMASK(23, 16))
#define ICPU_MEMCTRL_TIMING1_BANK8_FAW_DLY_M              GENMASK(23, 16)
#define ICPU_MEMCTRL_TIMING1_BANK8_FAW_DLY_X(x)           (((x) & GENMASK(23, 16)) >> 16)
#define ICPU_MEMCTRL_TIMING1_PRECH_TO_RAS_DLY(x)          (((x) << 12) & GENMASK(15, 12))
#define ICPU_MEMCTRL_TIMING1_PRECH_TO_RAS_DLY_M           GENMASK(15, 12)
#define ICPU_MEMCTRL_TIMING1_PRECH_TO_RAS_DLY_X(x)        (((x) & GENMASK(15, 12)) >> 12)
#define ICPU_MEMCTRL_TIMING1_RAS_TO_RAS_DLY(x)            (((x) << 8) & GENMASK(11, 8))
#define ICPU_MEMCTRL_TIMING1_RAS_TO_RAS_DLY_M             GENMASK(11, 8)
#define ICPU_MEMCTRL_TIMING1_RAS_TO_RAS_DLY_X(x)          (((x) & GENMASK(11, 8)) >> 8)
#define ICPU_MEMCTRL_TIMING1_RAS_TO_CAS_DLY(x)            (((x) << 4) & GENMASK(7, 4))
#define ICPU_MEMCTRL_TIMING1_RAS_TO_CAS_DLY_M             GENMASK(7, 4)
#define ICPU_MEMCTRL_TIMING1_RAS_TO_CAS_DLY_X(x)          (((x) & GENMASK(7, 4)) >> 4)
#define ICPU_MEMCTRL_TIMING1_WR_TO_RD_DLY(x)              ((x) & GENMASK(3, 0))
#define ICPU_MEMCTRL_TIMING1_WR_TO_RD_DLY_M               GENMASK(3, 0)

#define ICPU_MEMCTRL_TIMING2                              0x12c

#define ICPU_MEMCTRL_TIMING2_PRECH_ALL_DLY(x)             (((x) << 28) & GENMASK(31, 28))
#define ICPU_MEMCTRL_TIMING2_PRECH_ALL_DLY_M              GENMASK(31, 28)
#define ICPU_MEMCTRL_TIMING2_PRECH_ALL_DLY_X(x)           (((x) & GENMASK(31, 28)) >> 28)
#define ICPU_MEMCTRL_TIMING2_MDSET_DLY(x)                 (((x) << 24) & GENMASK(27, 24))
#define ICPU_MEMCTRL_TIMING2_MDSET_DLY_M                  GENMASK(27, 24)
#define ICPU_MEMCTRL_TIMING2_MDSET_DLY_X(x)               (((x) & GENMASK(27, 24)) >> 24)
#define ICPU_MEMCTRL_TIMING2_REF_DLY(x)                   (((x) << 16) & GENMASK(23, 16))
#define ICPU_MEMCTRL_TIMING2_REF_DLY_M                    GENMASK(23, 16)
#define ICPU_MEMCTRL_TIMING2_REF_DLY_X(x)                 (((x) & GENMASK(23, 16)) >> 16)
#define ICPU_MEMCTRL_TIMING2_INIT_DLY(x)                  ((x) & GENMASK(15, 0))
#define ICPU_MEMCTRL_TIMING2_INIT_DLY_M                   GENMASK(15, 0)

#define ICPU_MEMCTRL_TIMING3                              0x130

#define ICPU_MEMCTRL_TIMING3_RMW_DLY(x)                   (((x) << 16) & GENMASK(19, 16))
#define ICPU_MEMCTRL_TIMING3_RMW_DLY_M                    GENMASK(19, 16)
#define ICPU_MEMCTRL_TIMING3_RMW_DLY_X(x)                 (((x) & GENMASK(19, 16)) >> 16)
#define ICPU_MEMCTRL_TIMING3_ODT_RD_DLY(x)                (((x) << 12) & GENMASK(15, 12))
#define ICPU_MEMCTRL_TIMING3_ODT_RD_DLY_M                 GENMASK(15, 12)
#define ICPU_MEMCTRL_TIMING3_ODT_RD_DLY_X(x)              (((x) & GENMASK(15, 12)) >> 12)
#define ICPU_MEMCTRL_TIMING3_ODT_WR_DLY(x)                (((x) << 8) & GENMASK(11, 8))
#define ICPU_MEMCTRL_TIMING3_ODT_WR_DLY_M                 GENMASK(11, 8)
#define ICPU_MEMCTRL_TIMING3_ODT_WR_DLY_X(x)              (((x) & GENMASK(11, 8)) >> 8)
#define ICPU_MEMCTRL_TIMING3_LOCAL_ODT_RD_DLY(x)          (((x) << 4) & GENMASK(7, 4))
#define ICPU_MEMCTRL_TIMING3_LOCAL_ODT_RD_DLY_M           GENMASK(7, 4)
#define ICPU_MEMCTRL_TIMING3_LOCAL_ODT_RD_DLY_X(x)        (((x) & GENMASK(7, 4)) >> 4)
#define ICPU_MEMCTRL_TIMING3_WR_TO_RD_CS_CHANGE_DLY(x)    ((x) & GENMASK(3, 0))
#define ICPU_MEMCTRL_TIMING3_WR_TO_RD_CS_CHANGE_DLY_M     GENMASK(3, 0)

#define ICPU_MEMCTRL_MR0_VAL                              0x138

#define ICPU_MEMCTRL_MR1_VAL                              0x13c

#define ICPU_MEMCTRL_MR2_VAL                              0x140

#define ICPU_MEMCTRL_MR3_VAL                              0x144

#define ICPU_MEMCTRL_TERMRES_CTRL                         0x148

#define ICPU_MEMCTRL_TERMRES_CTRL_ODT_RD_EXT              BIT(11)
#define ICPU_MEMCTRL_TERMRES_CTRL_ODT_RD_ENA(x)           (((x) << 7) & GENMASK(10, 7))
#define ICPU_MEMCTRL_TERMRES_CTRL_ODT_RD_ENA_M            GENMASK(10, 7)
#define ICPU_MEMCTRL_TERMRES_CTRL_ODT_RD_ENA_X(x)         (((x) & GENMASK(10, 7)) >> 7)
#define ICPU_MEMCTRL_TERMRES_CTRL_ODT_WR_EXT              BIT(6)
#define ICPU_MEMCTRL_TERMRES_CTRL_ODT_WR_ENA(x)           (((x) << 2) & GENMASK(5, 2))
#define ICPU_MEMCTRL_TERMRES_CTRL_ODT_WR_ENA_M            GENMASK(5, 2)
#define ICPU_MEMCTRL_TERMRES_CTRL_ODT_WR_ENA_X(x)         (((x) & GENMASK(5, 2)) >> 2)
#define ICPU_MEMCTRL_TERMRES_CTRL_LOCAL_ODT_RD_EXT        BIT(1)
#define ICPU_MEMCTRL_TERMRES_CTRL_LOCAL_ODT_RD_ENA        BIT(0)

#define ICPU_MEMCTRL_DQS_DLY(x) (0x150 + 0x4 * (x))
#define ICPU_MEMCTRL_DQS_DLY_RSZ                          0x4

#define ICPU_MEMCTRL_DQS_DLY_TRAIN_DQ_ENA                 BIT(11)
#define ICPU_MEMCTRL_DQS_DLY_DQS_DLY_TRM1(x)              (((x) << 8) & GENMASK(10, 8))
#define ICPU_MEMCTRL_DQS_DLY_DQS_DLY_TRM1_M               GENMASK(10, 8)
#define ICPU_MEMCTRL_DQS_DLY_DQS_DLY_TRM1_X(x)            (((x) & GENMASK(10, 8)) >> 8)
#define ICPU_MEMCTRL_DQS_DLY_DQS_DLY_TRM0(x)              (((x) << 5) & GENMASK(7, 5))
#define ICPU_MEMCTRL_DQS_DLY_DQS_DLY_TRM0_M               GENMASK(7, 5)
#define ICPU_MEMCTRL_DQS_DLY_DQS_DLY_TRM0_X(x)            (((x) & GENMASK(7, 5)) >> 5)
#define ICPU_MEMCTRL_DQS_DLY_DQS_DLY(x)                   ((x) & GENMASK(4, 0))
#define ICPU_MEMCTRL_DQS_DLY_DQS_DLY_M                    GENMASK(4, 0)

#define ICPU_MEMPHY_CFG                                   0x160

#define ICPU_MEMPHY_CFG_PHY_FLUSH_DIS                     BIT(10)
#define ICPU_MEMPHY_CFG_PHY_RD_ADJ_DIS                    BIT(9)
#define ICPU_MEMPHY_CFG_PHY_DQS_EXT                       BIT(8)
#define ICPU_MEMPHY_CFG_PHY_FIFO_RST                      BIT(7)
#define ICPU_MEMPHY_CFG_PHY_DLL_BL_RST                    BIT(6)
#define ICPU_MEMPHY_CFG_PHY_DLL_CL_RST                    BIT(5)
#define ICPU_MEMPHY_CFG_PHY_ODT_OE                        BIT(4)
#define ICPU_MEMPHY_CFG_PHY_CK_OE                         BIT(3)
#define ICPU_MEMPHY_CFG_PHY_CL_OE                         BIT(2)
#define ICPU_MEMPHY_CFG_PHY_SSTL_ENA                      BIT(1)
#define ICPU_MEMPHY_CFG_PHY_RST                           BIT(0)

#define ICPU_MEMPHY_ZCAL                                  0x188

#define ICPU_MEMPHY_ZCAL_ZCAL_CLK_SEL                     BIT(9)
#define ICPU_MEMPHY_ZCAL_ZCAL_PROG_ODT(x)                 (((x) << 5) & GENMASK(8, 5))
#define ICPU_MEMPHY_ZCAL_ZCAL_PROG_ODT_M                  GENMASK(8, 5)
#define ICPU_MEMPHY_ZCAL_ZCAL_PROG_ODT_X(x)               (((x) & GENMASK(8, 5)) >> 5)
#define ICPU_MEMPHY_ZCAL_ZCAL_PROG(x)                     (((x) << 1) & GENMASK(4, 1))
#define ICPU_MEMPHY_ZCAL_ZCAL_PROG_M                      GENMASK(4, 1)
#define ICPU_MEMPHY_ZCAL_ZCAL_PROG_X(x)                   (((x) & GENMASK(4, 1)) >> 1)
#define ICPU_MEMPHY_ZCAL_ZCAL_ENA                         BIT(0)

#define ICPU_MEMPHY_ZCAL_STAT                             0x18c

#define ICPU_MEMPHY_ZCAL_STAT_ZCAL_ZCTRL(x)               (((x) << 12) & GENMASK(31, 12))
#define ICPU_MEMPHY_ZCAL_STAT_ZCAL_ZCTRL_M                GENMASK(31, 12)
#define ICPU_MEMPHY_ZCAL_STAT_ZCAL_ZCTRL_X(x)             (((x) & GENMASK(31, 12)) >> 12)
#define ICPU_MEMPHY_ZCAL_STAT_ZCAL_STAT_ODTPU(x)          (((x) << 8) & GENMASK(9, 8))
#define ICPU_MEMPHY_ZCAL_STAT_ZCAL_STAT_ODTPU_M           GENMASK(9, 8)
#define ICPU_MEMPHY_ZCAL_STAT_ZCAL_STAT_ODTPU_X(x)        (((x) & GENMASK(9, 8)) >> 8)
#define ICPU_MEMPHY_ZCAL_STAT_ZCAL_STAT_ODTPD(x)          (((x) << 6) & GENMASK(7, 6))
#define ICPU_MEMPHY_ZCAL_STAT_ZCAL_STAT_ODTPD_M           GENMASK(7, 6)
#define ICPU_MEMPHY_ZCAL_STAT_ZCAL_STAT_ODTPD_X(x)        (((x) & GENMASK(7, 6)) >> 6)
#define ICPU_MEMPHY_ZCAL_STAT_ZCAL_STAT_PU(x)             (((x) << 4) & GENMASK(5, 4))
#define ICPU_MEMPHY_ZCAL_STAT_ZCAL_STAT_PU_M              GENMASK(5, 4)
#define ICPU_MEMPHY_ZCAL_STAT_ZCAL_STAT_PU_X(x)           (((x) & GENMASK(5, 4)) >> 4)
#define ICPU_MEMPHY_ZCAL_STAT_ZCAL_STAT_PD(x)             (((x) << 2) & GENMASK(3, 2))
#define ICPU_MEMPHY_ZCAL_STAT_ZCAL_STAT_PD_M              GENMASK(3, 2)
#define ICPU_MEMPHY_ZCAL_STAT_ZCAL_STAT_PD_X(x)           (((x) & GENMASK(3, 2)) >> 2)
#define ICPU_MEMPHY_ZCAL_STAT_ZCAL_ERR                    BIT(1)
#define ICPU_MEMPHY_ZCAL_STAT_ZCAL_DONE                   BIT(0)
#endif
