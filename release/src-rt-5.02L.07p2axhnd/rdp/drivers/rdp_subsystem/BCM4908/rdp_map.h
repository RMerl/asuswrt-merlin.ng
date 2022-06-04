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

#ifndef __RDP_MAP_H
#define __RDP_MAP_H


#if defined(BDMF_SYSTEM_SIM)
#define RDP_PHYS_BASE 0x82200000
#define RDP_SIZE   0xfc460
#define FPM_BPM_PHYS_BASE 0x82c30000
#define FPM_BPM_SIZE 0x134
#else
#include "4908_map_part.h"
#endif

/*****************************************************************************************/
/* BBH Blocks offsets                                                                    */
/*****************************************************************************************/
#define BBH_TX_0_OFFSET (0xe8000)
#define BBH_TX_1_OFFSET	(0xea000)
#define BBH_TX_2_OFFSET	(0xec000)
#define BBH_TX_3_OFFSET	(0xee000)
#define BBH_RX_0_OFFSET	(0xde000)
#define BBH_RX_1_OFFSET	(0xde800)
#define BBH_RX_2_OFFSET	(0xdf000)
#define BBH_RX_3_OFFSET	(0xdf800)

/*****************************************************************************************/
/* BPM Blocks offsets   - in 4908 BPM is outside the RDP block (FPM)                     */
/*****************************************************************************************/
#define FPM_BPM_PHYS_ADDR (FPM_BPM_PHYS_BASE)
#define FPM_BPM_PHYS_SIZE (FPM_BPM_SIZE)

/*****************************************************************************************/
/* SBPM Blocks offsets                                                                   */
/*****************************************************************************************/
#define SBPM_BLOCK_OFFSET (0xc8000)
/*****************************************************************************************/
/* DMA Blocks offsets                                                                    */
/*****************************************************************************************/
#define DMA_REGS_0_OFFSET (0xd1000)
#define DMA_REGS_1_OFFSET (0xd1800)
#define DMA_REGS_0_MEM_SET (0xd1098)
#define DMA_REGS_1_MEM_SET (0xd1898)

/*****************************************************************************************/
/* IH Blocks offsets                                                                     */
/*****************************************************************************************/
#define IH_REGS_OFFSET (0xd0000)

/*****************************************************************************************/
/* PSRAM Blocks offsets                                                                  */
/*****************************************************************************************/
#define PSRAM_BLOCK_OFFSET (0xa0000)
/*****************************************************************************************/
/* RUNNER Blocks offsets                                                                 */
/*****************************************************************************************/
#define RUNNER_COMMON_0_OFFSET (0x00000)
#define RUNNER_PRIVATE_0_OFFSET (0x10000)
#define RUNNER_INST_MAIN_0_OFFSET (0x20000)
#define RUNNER_CNTXT_MAIN_0_OFFSET (0x28000)
#define RUNNER_PRED_MAIN_0_OFFSET (0x2c000)
#define RUNNER_INST_PICO_0_OFFSET (0x30000)
#define RUNNER_CNTXT_PICO_0_OFFSET (0x38000)
#define RUNNER_PRED_PICO_0_OFFSET (0x3c000)
#define RUNNER_REGS_0_OFFSET (0x99000)
#define RUNNER_COMMON_1_OFFSET (0x40000)
#define RUNNER_PRIVATE_1_OFFSET (0x50000)
#define RUNNER_INST_MAIN_1_OFFSET (0x60000)
#define RUNNER_CNTXT_MAIN_1_OFFSET (0x68000)
#define RUNNER_PRED_MAIN_1_OFFSET (0x6c000)
#define RUNNER_INST_PICO_1_OFFSET (0x70000)
#define RUNNER_CNTXT_PICO_1_OFFSET (0x78000)
#define RUNNER_PRED_PICO_1_OFFSET (0x7c000)
#define RUNNER_REGS_1_OFFSET (0x9a000)
/*****************************************************************************************/
/* UBUS MASTER Blocks offsets                                                            */
/*****************************************************************************************/
#define UBUS_MASTER_1_RDP_UBUS_MASTER_BRDG_REG_EN (0xd2000)
#define UBUS_MASTER_2_RDP_UBUS_MASTER_BRDG_REG_EN (0xd2400)
#define UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_EN (0xd2800)
#define UBUS_MASTER_4_RDP_UBUS_MASTER_BRDG_REG_EN (0xd3c00)
#define RDP_UBUS_MASTER_BRDG_REG_HP_OFFSET (0xc)

/*****************************************************************************************/
/* NAT Cache Block offsets                                                            */
/*****************************************************************************************/
#define NATCACHE_RDP (0xfc000)

/*****************************************************************************************/
#define UNIMAC_CONF_RDP (0xd4000)
#define UNIMAC_MISC_RDP (0xdb800)
#define UNIMAC_MIB_RDP  (0xda000)
/* FPM Block offsets                                                            */
/*****************************************************************************************/
//#define FPM_BUF_SIZE_ADDR BCM_IO_ADDR(0x3a00040)
//#define FPM_BUF_SIZE_MASK 0x07000000
//#define FPM_BUF_SIZE_OFFSET 24
//#define FPM_BASE_ADDRESS_POOL0_ADDR BCM_IO_ADDR(0x3a00044)
//#define FPM_BASE_ADDRESS_POOL0_OFFSET 2
//#define FPM_BASE_ADDRESS_POOL1_ADDR BCM_IO_ADDR(0x3a00048)
//#define FPM_BASE_ADDRESS_POOL1_OFFSET 2


/*****************************************************************************************/
/* RNR_WKUP_CPUC Block offsets                                                            */
/*****************************************************************************************/
#define RNR_WKUP_CPUC_BASE 0x82101800
#define RNR_WKUP_CPUC_SIZE 0x188

#endif
