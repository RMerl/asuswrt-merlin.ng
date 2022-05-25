/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2017:DUAL/GPL:standard

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

/*
 *  Created on: Sep 2017
 *      Author: li.xu@broadcom.com
 */

#ifndef MERLIN_HOST_H
#define MERLIN_HOST_H

#define REG_WRITE_32(reg, val) do{*(volatile uint32_t *)(reg) = (val);} while(0)
#define REG_READ_32(reg, var) do{(var) = *(volatile uint32_t *)(reg);} while(0)
#define REG_DIR_WRITE_32(reg, val) REG_WRITE_32(bcm_dev_phy2vir(reg), val)
#define REG_DIR_READ_32(reg, var) REG_READ_32(bcm_dev_phy2vir(reg), var)

#define SERDES_CORE_BASE            0x000000
#define SERDES_CORE_OFFSET          0x040000
#define SERDES_INDIR_ACC_CNTRL_BASE 0x3ff5f0
#define SERDES_MISC_REG_BASE        0x3ff500
#define SERDES_REG_OFFSET           0x000100

#define MERLIN_REVID_RBUS           0x0000
#define MERLIN_CTRL_RBUS            0x0004
#define MERLIN_STATUS_RBUS          0x0008
#define MERLIN_INDIR_ACC_CNTRL_RBUS 0x0010
#define MERLIN_INDIR_ACC_ADDR_RBUS  0x0014
#define MERLIN_INDIR_ACC_MASK_RBUS  0x0018
#define MERLIN_INDIR_ACC_CNTRL 0x3ff5f0
#define MERLIN_INDIR_ACC_ADDR  0x3ff504
#define MERLIN_INDIR_ACC_MASK  0x3ff508

#define MERLIN_REVID           0x3ff51c
#define MERLIN_CTRL            0x3ff50c
#define MERLIN_STATUS          0x3ff510
#define MERLIN_AN_LINK_STATUS  0x3ff520
#define MERLIN_SERDES_0_MISC_STATUS 0x3ff524
    #define MERLIN_MISC_SPEED_10G   (0xf<<20)
    #define MERLIN_MISC_SPEED_5G    (0xf<<16)
    #define MERLIN_MISC_SPEED_2P5G  (0xf<<12)
    #define MERLIN_MISC_SPEED_1G    (0xf<<8)
    #define MERLIN_MISC_SPEED_100M  (0xf<<4)
    #define MERLIN_MISC_SPEED_10M   (0xf<<0)


#define XLMAC_REG_0_XLMAC_0_CONFIG_0 	0x83400000+0x3f3020
#define XLMAC_REG_0_XLMAC_0_CONFIG_1 	0x83400000+0x3f7020
#define REG_XPORT_CNTR_0		0x83400000+0x3f2000
#define REG_XPORT_CNTR_1		0x83400000+0x3f6000
#define XPORT_BASE_0			0x83400000+0x3f0000
#define XPORT_BASE_1			0x83400000+0x3f4000
#define XPORT_RMT_LPBK_CNTRL_0			0x83400000+0x3f3038
#define XPORT0_PORT_0_MIB_RSV_MASK		0x83400000+0x3f303c
#define XPORT0_PORT_1_MIB_RSV_MASK		0x83400000+0x3f3040
#define XPORT0_PORT_2_MIB_RSV_MASK		0x83400000+0x3f3044
#define XPORT0_PORT_3_MIB_RSV_MASK		0x83400000+0x3f3048
#define XPORT_RMT_LPBK_CNTRL_1			0x83400000+0x3f7038
#define XPORT1_PORT_0_MIB_RSV_MASK		0x83400000+0x3f703c
#define XPORT1_PORT_1_MIB_RSV_MASK		0x83400000+0x3f7040
#define XPORT1_PORT_2_MIB_RSV_MASK		0x83400000+0x3f7044
#define XPORT1_PORT_3_MIB_RSV_MASK		0x83400000+0x3f7048


#define XLMAC_CORE_0_REG_BASE           0x0000
#define XLMAC_REG_0_XLMAC_0_DIR_ACC_DATA_WRITE 0x3000
#define XLMAC_REG_0_XLMAC_0_DIR_ACC_DATA_READ  0x3004
#define XLMAC_CTRL_index 0 
#define XLMAC_MODE_index 1
#define XLMAC_SPARE0_index 2
#define XLMAC_SPARE1_index 3
#define XLMAC_TX_CTRL_index 4
#define XLMAC_TX_MAC_SA_index 5
#define XLMAC_RX_CTRL_index 6 
#define XLMAC_RX_MAC_SA_index 7
#define XLMAC_RX_MAX_SIZE_index 8 
#define XLMAC_RX_VLAN_TAG_index 9
#define XLMAC_RX_LSS_CTRL_index 10
#define XLMAC_RX_LSS_STATUS_index 11
#define XLMAC_CLEAR_RX_LSS_STATUS_index 12
#define XLMAC_PAUSE_CTRL_index  13
#define XLMAC_PFC_CTRL_index 14
#define XLMAC_PFC_TYPE_index 15
#define XLMAC_PFC_OPCODE_index 16 
#define XLMAC_PFC_DA_index 17
#define XLMAC_LLFC_CTRL_index 18
#define XLMAC_TX_LLFC_MSG_FIELDS_index 19 
#define XLMAC_RX_LLFC_MSG_FIELDS_index 20
#define XLMAC_TX_TIMESTMAP_FIFO_DATA_index 21
#define XLMAC_TX_TIMESTAMP_FIFO_STATUS_index 22
#define XLMAC_FIFO_STATUS_index 23
#define XLMAC_CLEAR_FIFO_STATUS_index 24
#define XLMAC_LAG_FAILOVER_STATUS_index 25 
#define XLMAC_EEE_CTRL_index 26 
#define XLMAC_EEE_TIMERS_index 27 
#define XLMAC_EEE_1_SEC_LINK_STATUS_TIMER_index 28 
#define XLMAC_HIGIG_HDR_0_index 29 
#define XLMAC_HIGIG_HDR_1_index 30 
#define XLMAC_GMII_EEE_CTRL_index 31
#define XLMAC_TIMESTAMP_ADJUST_index 32
#define XLMAC_TIMESTAMP_BYTE_ADJUST_index 33
#define XLMAC_TX_CRC_CORRUPT_CTRL_index 34
#define XLMAC_E2E_CTRL_index 35 
#define XLMAC_E2ECC_MODULE_HDR_0_index 36
#define XLMAC_E2ECC_MODULE_HDR_1_index 37 
#define XLMAC_E2ECC_DATA_HDR_0_index 38 
#define XLMAC_E2ECC_DATA_HDR_1_index 39 
#define XLMAC_E2EFC_MODULE_HDR_0_index 40
#define XLMAC_E2EFC_MODULE_HDR_1_index 41
#define XLMAC_E2EFC_DATA_HDR_0_index 42
#define XLMAC_E2EFC_DATA_HDR_1_index 43
#define XLMAC_TXFIFO_CELL_CNT_index 44
#define XLMAC_TXFIFO_CELL_REQ_CNT_index  45
#define XLMAC_CLEAR_ECC_STATUS_index 52
#define XLMAC_INTR_STATUS_index 53
#define XLMAC_INTR_ENABLE_index 54
#define XLMAC_VERSION_ID_index 55 
#define REG_MAB_0_CNTRL             0x3300
#define REG_MAB_0_TX_WRR_CTRL       0x3304


#define ETH_PHY_TOP_REG_R2PMI_LP_BCAST_MODE_CNTRL 0x3ff000
//cross check with vlib/host_sim_model.v
#define MERLIN_SERDES_CTRL_ISO_ENABLE_OFFSET   27
#define MERLIN_SERDES_CTRL_TESTSEL_OFFSET      21
#define MERLIN_SERDES_CTRL_TEST_EN_OFFSET      20
#define MERLIN_SERDES_CTRL_LN_OFFSET_OFFSET    11
#define MERLIN_SERDES_CTRL_PRTAD_OFFSET        6
#define MERLIN_SERDES_CTRL_REFSEL_OFFSET       3
#define MERLIN_SERDES_CTRL_RESET_OFFSET        2
#define MERLIN_SERDES_CTRL_REFCLK_RESET_OFFSET 1
#define MERLIN_SERDES_CTRL_IDDQ_OFFSET         0

#define MERLIN_SERDES_CTRL_ISO_ENABLE_MASK    0x1
#define MERLIN_SERDES_CTRL_TESTSEL_MASK       0x7  //3-bit
#define MERLIN_SERDES_CTRL_TEST_EN_MASK       0x1
#define MERLIN_SERDES_CTRL_LN_OFFSET_MASK     0x1f //5-bit
#define MERLIN_SERDES_CTRL_PRTAD_MASK         0x1f //5-bit
#define MERLIN_SERDES_CTRL_REFSEL_MASK        0x7  //3-bit
#define MERLIN_SERDES_CTRL_RESET_MASK         0x1
#define MERLIN_SERDES_CTRL_REFCLK_RESET_MASK  0x1
#define MERLIN_SERDES_CTRL_IDDQ_MASK          0x1
//

#define MERLIN_PMI_BC_ADDR     0x1F
#define MERLIN_PMI_BC_CNTRL    (MERLIN_PMI_BC_ADDR<<8)+0x00

#endif //MERLIN_HOST_H
