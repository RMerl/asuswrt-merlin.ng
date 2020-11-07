/*
    <:copyright-BRCM:2013:DUAL/GPL:standard
    
       Copyright (c) 2013 Broadcom 
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

#ifndef __96838_RDP_MAP_H
#define __96838_RDP_MAP_H

/*****************************************************************************************/
/* BBH Blocks offsets                                                                    */
/*****************************************************************************************/
#define BBH_TX_0_OFFSET				( 0x130e8000 )
#define BBH_TX_1_OFFSET				( 0x130ea000 )
#define BBH_TX_2_OFFSET				( 0x130ec000 )
#define BBH_TX_3_OFFSET				( 0x130ee000 )
#define BBH_TX_4_OFFSET				( 0x130f0000 )
#define BBH_TX_5_OFFSET				( 0x130f2000 )
#define BBH_TX_6_OFFSET				( 0x130f4000 )
#define BBH_TX_7_OFFSET				( 0x130f6000 )
#define BBH_RX_0_OFFSET				( 0x130de000 )
#define BBH_RX_1_OFFSET				( 0x130de800 )
#define BBH_RX_2_OFFSET				( 0x130df000 )
#define BBH_RX_3_OFFSET				( 0x130df800 )
#define BBH_RX_4_OFFSET				( 0x130e0000 )
#define BBH_RX_5_OFFSET				( 0x130e0800 )
#define BBH_RX_6_OFFSET				( 0x130e1000 )
#define BBH_RX_7_OFFSET				( 0x130e2000 )
/*****************************************************************************************/
/* BPM Blocks offsets                                                                    */
/*****************************************************************************************/
#define BPM_MODULE_OFFSET			( 0x130c4000 )
/*****************************************************************************************/
/* SBPM Blocks offsets                                                                   */
/*****************************************************************************************/
#define SBPM_BLOCK_OFFSET			( 0x130c8000 )
/*****************************************************************************************/
/* GPON ClkOut Configuration Blocks offsets                                              */
/*****************************************************************************************/
#define GPONCLKOUT_CFG_REGS_OFFSET ( 0x130cc000 )
/*****************************************************************************************/
/* DMA Blocks offsets                                                                    */
/*****************************************************************************************/
#define DMA_REGS_0_OFFSET			( 0x130d1000 )
#define DMA_REGS_1_OFFSET			( 0x130d1800 )
#define DMA_REGS_0_MEM_SET          ( 0x130d1098 )
/*****************************************************************************************/
/* GPON Blocks offsets                                                                   */
/*****************************************************************************************/
#define GPON_RX_OFFSET				( 0x130f9000 )
#define GPON_TX_OFFSET				( 0x130fa000 )
/*****************************************************************************************/
/* IH Blocks offsets                                                                     */
/*****************************************************************************************/
#define IH_REGS_OFFSET				( 0x130d0000 )
/*****************************************************************************************/
/* MS1588 Blocks offsets                                                                 */
/*****************************************************************************************/
#define MS1588_MASTER_OFFSET		( 0x130d3d00 )
#define MS1588_LOCAL_TS_OFFSET		( 0x130d3e00 )
/*****************************************************************************************/
/* PSRAM Blocks offsets                                                                  */
/*****************************************************************************************/
#define PSRAM_BLOCK_OFFSET			( 0x130a0000  )
/*****************************************************************************************/
/* RUNNER Blocks offsets                                                                 */
/*****************************************************************************************/
#define RUNNER_COMMON_0_OFFSET		( 0x13000000 )
#define RUNNER_COMMON_1_OFFSET		( 0x13040000 )
#define RUNNER_PRIVATE_0_OFFSET		( 0x13010000 )
#define RUNNER_PRIVATE_1_OFFSET		( 0x13050000 )
#define RUNNER_INST_MAIN_0_OFFSET	( 0x13020000 )
#define RUNNER_INST_MAIN_1_OFFSET	( 0x13060000 )
#define RUNNER_CNTXT_MAIN_0_OFFSET	( 0x13028000 )
#define RUNNER_CNTXT_MAIN_1_OFFSET	( 0x13068000 )
#define RUNNER_PRED_MAIN_0_OFFSET	( 0x1302c000 )
#define RUNNER_PRED_MAIN_1_OFFSET	( 0x1306c000 )
#define RUNNER_INST_PICO_0_OFFSET	( 0x13030000 )
#define RUNNER_INST_PICO_1_OFFSET	( 0x13070000 )
#define RUNNER_CNTXT_PICO_0_OFFSET	( 0x13038000 )
#define RUNNER_CNTXT_PICO_1_OFFSET	( 0x13078000 )
#define RUNNER_PRED_PICO_0_OFFSET	( 0x1303c000 )
#define RUNNER_PRED_PICO_1_OFFSET	( 0x1307c000 )
#define RUNNER_REGS_0_OFFSET		( 0x13099000 )
#define RUNNER_REGS_1_OFFSET		( 0x1309a000 )
/*****************************************************************************************/
/* UBUS MASTER Blocks offsets                                                            */
/*****************************************************************************************/
#define UBUS_MASTER_1_RDP_UBUS_MASTER_BRDG_REG_EN ( 0x130d2000 )
#define UBUS_MASTER_2_RDP_UBUS_MASTER_BRDG_REG_EN ( 0x130d2400 )
#define UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_EN ( 0x130d2800 )
#define UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_HP ( 0x130d280c )
#define UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_HP ( 0x130d280c )

#define EGPHY_BASE                                ( 0x130d3000 )
#define EGPHY_UBUS_MISC_EGPHY_GPHY_OUT      (EGPHY_BASE + 0x00)
#define EGPHY_UBUS_MISC_EGPHY_RGMII_OUT     (EGPHY_BASE + 0x04)
#define EGPHY_UBUS_MISC_EGPHY_GPHY_IN       (EGPHY_BASE + 0x08)
#define EGPHY_UBUS_MISC_EGPHY_GPHY_IN2      (EGPHY_BASE + 0x0c)
#define EGPHY_UBUS_MISC_EGPHY_ECO_0         (EGPHY_BASE + 0x10)
#define EGPHY_UBUS_MISC_EGPHY_ECO_1         (EGPHY_BASE + 0x14)
#define EGPHY_RGMII_0_MUX_CONFIG            (EGPHY_BASE + 0x18)
#define EGPHY_EPHY_PHY_AD                   (EGPHY_BASE + 0x1c)
#define EGPHY_EPHY_PWR_MGNT                 (EGPHY_BASE + 0x20)
#define EGPHY_EPHY_RESET_CNTRL              (EGPHY_BASE + 0x24)
#define EGPHY_EPHY_REF_CLK_SEL              (EGPHY_BASE + 0x28)
#define EGPHY_EPHY_TEST_CNTRL               (EGPHY_BASE + 0x2c)
#define EGPHY_EPHY_PHY_SELECT               (EGPHY_BASE + 0x30)
#define EGPHY_RGMII_0_CNTRL                 (EGPHY_BASE + 0x38)
#define EGPHY_RGMII_0_IB_STATUS             (EGPHY_BASE + 0x3c)
#define EGPHY_RGMII_0_RX_CLOCK_DELAY_CNTRL  (EGPHY_BASE + 0x40)
#define EGPHY_RGMII_0_ATE_RX_CNTRL          (EGPHY_BASE + 0x44)
#define EGPHY_RGMII_0_ATE_RX_STATUS         (EGPHY_BASE + 0x48)
#define EGPHY_RGMII_0_ATE_TX_CNTRL_0        (EGPHY_BASE + 0x4c)
#define EGPHY_RGMII_0_ATE_TX_CNTRL_1        (EGPHY_BASE + 0x50)
#define EGPHY_SINGLE_SERDES_REV             (EGPHY_BASE + 0x54)
#define EGPHY_SINGLE_SERDES_STAT            (EGPHY_BASE + 0x58)
#define EGPHY_SINGLE_SERDES_CNTRL           (EGPHY_BASE + 0x5c)
#define EGPHY_SINGLE_SERDES_APD_CNTRL       (EGPHY_BASE + 0x60)
#define EGPHY_SINGLE_SERDES_APD_FSM_CNTRL   (EGPHY_BASE + 0x64)
#define EGPHY_RGMII_0_TX_DELAY_CNTRL_0      (EGPHY_BASE + 0x68)
#define EGPHY_RGMII_0_TX_DELAY_CNTRL_1      (EGPHY_BASE + 0x6c)
#define EGPHY_RGMII_0_RX_DELAY_CNTRL_0      (EGPHY_BASE + 0x70)
#define EGPHY_RGMII_0_RX_DELAY_CNTRL_1      (EGPHY_BASE + 0x74)
#define EGPHY_RGMII_0_RX_DELAY_CNTRL_2      (EGPHY_BASE + 0x78)

#endif
