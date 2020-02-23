/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
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

#ifndef DRV_DMA_H_INCLUDED
#define DRV_DMA_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif


/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/
#include "xrdp_drv_dma_ag.h"
#if defined(BCM6836) || defined(BCM6846) || defined(BCM6856) || defined(BCM63158)
#define NUM_OF_PERIPHERALS_PER_DMA 7
#else
#define NUM_OF_PERIPHERALS_PER_DMA 6
#endif

/******************************************************************************/
/*                                                                            */
/* Types and values definitions                                               */
/*                                                                            */
/******************************************************************************/

#define DMA0_U_THRESH_IN_BBH_ID_XLMAC1_1_RGMII_VALUE  ( 0x3 )
#define DMA0_U_THRESH_IN_BBH_ID_XLMAC1_2_RGMII_VALUE  ( 0x3 )
#define DMA0_U_THRESH_IN_BBH_ID_PON_VALUE             ( 0x8 )
#define DMA0_U_THRESH_IN_BBH_ID_DSL_VALUE             ( 0x8 )
#define DMA0_U_THRESH_IN_BBH_ID_XLMAC0_3_1G_VALUE     ( 0x3 )
#define DMA1_U_THRESH_IN_BBH_ID_XLMAC0_1_2p5G_VALUE   ( 0x3 )
#define DMA1_U_THRESH_IN_BBH_ID_XLMAC0_2_1G_VALUE     ( 0x3 )
#define DMA1_U_THRESH_IN_BBH_ID_XLMAC1_0_RGMII_VALUE  ( 0x3 )
#define DMA1_U_THRESH_IN_BBH_ID_XLMAC0_0_10G_VALUE    ( 0x8 )

#define SDMA0_U_THRESH_IN_BBH_ID_XLMAC1_1_RGMII_VALUE  ( 0x3 )
#define SDMA0_U_THRESH_IN_BBH_ID_XLMAC1_2_RGMII_VALUE  ( 0x3 )
#define SDMA0_U_THRESH_IN_BBH_ID_PON_VALUE             ( 0x8 )
#define SDMA0_U_THRESH_IN_BBH_ID_DSL_VALUE             ( 0x8 )
#define SDMA0_U_THRESH_IN_BBH_ID_XLMAC0_3_1G_VALUE     ( 0x3 )
#define SDMA1_U_THRESH_IN_BBH_ID_XLMAC0_1_2p5G_VALUE   ( 0x3 )
#define SDMA1_U_THRESH_IN_BBH_ID_XLMAC0_2_1G_VALUE     ( 0x3 )
#define SDMA1_U_THRESH_IN_BBH_ID_XLMAC0_0_10G_VALUE    ( 0x8 )
#define SDMA1_U_THRESH_IN_BBH_ID_XLMAC1_0_RGMII_VALUE  ( 0x3 )

#define DMA0_U_THRESH_OUT_BBH_ID_XLMAC1_1_RGMII_VALUE  ( 0x2 )
#define DMA0_U_THRESH_OUT_BBH_ID_XLMAC1_2_RGMII_VALUE  ( 0x2 )
#define DMA0_U_THRESH_OUT_BBH_ID_PON_VALUE             ( 0x6 )
#define DMA0_U_THRESH_OUT_BBH_ID_DSL_VALUE             ( 0x6 )
#define DMA0_U_THRESH_OUT_BBH_ID_XLMAC0_3_1G_VALUE     ( 0x2 )
#define DMA1_U_THRESH_OUT_BBH_ID_XLMAC0_1_2p5G_VALUE   ( 0x2 )
#define DMA1_U_THRESH_OUT_BBH_ID_XLMAC0_2_1G_VALUE     ( 0x2 )
#define DMA1_U_THRESH_OUT_BBH_ID_XLMAC1_0_RGMII_VALUE  ( 0x2 )
#define DMA1_U_THRESH_OUT_BBH_ID_XLMAC0_0_10G_VALUE    ( 0x6 )

#define SDMA0_U_THRESH_OUT_BBH_ID_XLMAC1_1_RGMII_VALUE  ( 0x2 )
#define SDMA0_U_THRESH_OUT_BBH_ID_XLMAC1_2_RGMII_VALUE  ( 0x2 )
#define SDMA0_U_THRESH_OUT_BBH_ID_PON_VALUE             ( 0x6 )
#define SDMA0_U_THRESH_OUT_BBH_ID_DSL_VALUE             ( 0x6 )
#define SDMA0_U_THRESH_OUT_BBH_ID_XLMAC0_3_1G_VALUE     ( 0x2 )
#define SDMA1_U_THRESH_OUT_BBH_ID_XLMAC0_1_2p5G_VALUE   ( 0x2 )
#define SDMA1_U_THRESH_OUT_BBH_ID_XLMAC0_2_1G_VALUE     ( 0x2 )
#define SDMA1_U_THRESH_OUT_BBH_ID_XLMAC0_0_10G_VALUE    ( 0x6 )
#define SDMA1_U_THRESH_OUT_BBH_ID_XLMAC1_0_RGMII_VALUE  ( 0x2 )

#define DMA0_STRICT_PRI_RX_BBH_ID_XLMAC0_1_2p5G_VALUE   ( 0x4 )
#define DMA0_STRICT_PRI_RX_BBH_ID_XLMAC1_1_RGMII_VALUE  ( 0x4 )
#define DMA0_STRICT_PRI_RX_BBH_ID_XLMAC1_2_RGMII_VALUE  ( 0x4 )
#define DMA0_STRICT_PRI_RX_BBH_ID_PON_VALUE             ( 0x4 )
#define DMA0_STRICT_PRI_RX_BBH_ID_DSL_VALUE             ( 0x4 )
#define DMA1_STRICT_PRI_RX_BBH_ID_XLMAC0_0_10G_VALUE    ( 0x4 )
#define DMA1_STRICT_PRI_RX_BBH_ID_XLMAC0_2_1G_VALUE     ( 0x4 )
#define DMA1_STRICT_PRI_RX_BBH_ID_XLMAC0_3_1G_VALUE     ( 0x4 )
#define DMA1_STRICT_PRI_RX_BBH_ID_XLMAC1_0_RGMII_VALUE  ( 0x4 )

#define SDMA0_STRICT_PRI_RX_BBH_ID_XLMAC0_1_2p5G_VALUE   ( 0x4 )
#define SDMA0_STRICT_PRI_RX_BBH_ID_XLMAC1_1_RGMII_VALUE  ( 0x4 )
#define SDMA0_STRICT_PRI_RX_BBH_ID_XLMAC1_2_RGMII_VALUE  ( 0x4 )
#define SDMA0_STRICT_PRI_RX_BBH_ID_PON_VALUE             ( 0x4 )
#define SDMA0_STRICT_PRI_RX_BBH_ID_DSL_VALUE             ( 0x4 )
#define SDMA1_STRICT_PRI_RX_BBH_ID_XLMAC0_0_10G_VALUE    ( 0x4 )
#define SDMA1_STRICT_PRI_RX_BBH_ID_XLMAC0_2_1G_VALUE     ( 0x4 )
#define SDMA1_STRICT_PRI_RX_BBH_ID_XLMAC0_3_1G_VALUE     ( 0x4 )
#define SDMA1_STRICT_PRI_RX_BBH_ID_XLMAC1_0_RGMII_VALUE  ( 0x4 )

#define DMA0_STRICT_PRI_TX_BBH_ID_XLMAC0_1_2p5G_VALUE   ( 0x8 )
#define DMA0_STRICT_PRI_TX_BBH_ID_XLMAC1_1_RGMII_VALUE  ( 0x8 )
#define DMA0_STRICT_PRI_TX_BBH_ID_XLMAC1_2_RGMII_VALUE  ( 0x8 )
#define DMA0_STRICT_PRI_TX_BBH_ID_PON_VALUE             ( 0x8 )
#define DMA0_STRICT_PRI_TX_BBH_ID_DSL_VALUE             ( 0x8 )
#define DMA1_STRICT_PRI_TX_BBH_ID_XLMAC0_0_10G_VALUE    ( 0x8 )
#define DMA1_STRICT_PRI_TX_BBH_ID_XLMAC0_2_1G_VALUE     ( 0x8 )
#define DMA1_STRICT_PRI_TX_BBH_ID_XLMAC0_3_1G_VALUE     ( 0x8 )
#define DMA1_STRICT_PRI_TX_BBH_ID_XLMAC1_0_RGMII_VALUE  ( 0x8 )

#define SDMA0_STRICT_PRI_TX_BBH_ID_XLMAC0_1_2p5G_VALUE   ( 0x8 )
#define SDMA0_STRICT_PRI_TX_BBH_ID_XLMAC1_1_RGMII_VALUE  ( 0x8 )
#define SDMA0_STRICT_PRI_TX_BBH_ID_XLMAC1_2_RGMII_VALUE  ( 0x8 )
#define SDMA0_STRICT_PRI_TX_BBH_ID_PON_VALUE             ( 0x8 )
#define SDMA0_STRICT_PRI_TX_BBH_ID_DSL_VALUE             ( 0x8 )
#define SDMA1_STRICT_PRI_TX_BBH_ID_XLMAC0_0_10G_VALUE    ( 0x8 )
#define SDMA1_STRICT_PRI_TX_BBH_ID_XLMAC0_2_1G_VALUE     ( 0x8 )
#define SDMA1_STRICT_PRI_TX_BBH_ID_XLMAC0_3_1G_VALUE     ( 0x8 )
#define SDMA1_STRICT_PRI_TX_BBH_ID_XLMAC1_0_RGMII_VALUE  ( 0x8 )

#define DMA0_RR_WEIGHT_RX_BBH_ID_XLMAC0_1_2p5G_VALUE   ( 0x0 )
#define DMA0_RR_WEIGHT_RX_BBH_ID_XLMAC1_1_RGMII_VALUE  ( 0x0 )
#define DMA0_RR_WEIGHT_RX_BBH_ID_XLMAC1_2_RGMII_VALUE  ( 0x0 )
#define DMA0_RR_WEIGHT_RX_BBH_ID_PON_VALUE             ( 0x0 )
#define DMA0_RR_WEIGHT_RX_BBH_ID_DSL_VALUE             ( 0x0 )
#define DMA1_RR_WEIGHT_RX_BBH_ID_XLMAC0_0_10G_VALUE    ( 0x0 )
#define DMA1_RR_WEIGHT_RX_BBH_ID_XLMAC0_2_1G_VALUE     ( 0x0 )
#define DMA1_RR_WEIGHT_RX_BBH_ID_XLMAC0_3_1G_VALUE     ( 0x0 )
#define DMA1_RR_WEIGHT_RX_BBH_ID_XLMAC1_0_RGMII_VALUE  ( 0x0 )

#define SDMA0_RR_WEIGHT_RX_BBH_ID_XLMAC0_1_2p5G_VALUE   ( 0x0 )
#define SDMA0_RR_WEIGHT_RX_BBH_ID_XLMAC1_1_RGMII_VALUE  ( 0x0 )
#define SDMA0_RR_WEIGHT_RX_BBH_ID_XLMAC1_2_RGMII_VALUE  ( 0x0 )
#define SDMA0_RR_WEIGHT_RX_BBH_ID_PON_VALUE             ( 0x0 )
#define SDMA0_RR_WEIGHT_RX_BBH_ID_DSL_VALUE             ( 0x0 )
#define SDMA1_RR_WEIGHT_RX_BBH_ID_XLMAC0_0_10G_VALUE    ( 0x0 )
#define SDMA1_RR_WEIGHT_RX_BBH_ID_XLMAC0_2_1G_VALUE     ( 0x0 )
#define SDMA1_RR_WEIGHT_RX_BBH_ID_XLMAC0_3_1G_VALUE     ( 0x0 )
#define SDMA1_RR_WEIGHT_RX_BBH_ID_XLMAC1_0_RGMII_VALUE  ( 0x0 )

#define DMA0_RR_WEIGHT_TX_BBH_ID_XLMAC0_1_2p5G_VALUE   ( 0x0 )
#define DMA0_RR_WEIGHT_TX_BBH_ID_XLMAC1_1_RGMII_VALUE  ( 0x0 )
#define DMA0_RR_WEIGHT_TX_BBH_ID_XLMAC1_2_RGMII_VALUE  ( 0x0 )
#define DMA0_RR_WEIGHT_TX_BBH_ID_PON_VALUE             ( 0x0 )
#define DMA0_RR_WEIGHT_TX_BBH_ID_DSL_VALUE             ( 0x0 )
#define DMA1_RR_WEIGHT_TX_BBH_ID_XLMAC0_0_10G_VALUE    ( 0x0 )
#define DMA1_RR_WEIGHT_TX_BBH_ID_XLMAC0_2_1G_VALUE     ( 0x0 )
#define DMA1_RR_WEIGHT_TX_BBH_ID_XLMAC0_3_1G_VALUE     ( 0x0 )
#define DMA1_RR_WEIGHT_TX_BBH_ID_XLMAC1_0_RGMII_VALUE  ( 0x0 )

#define SDMA0_RR_WEIGHT_TX_BBH_ID_XLMAC0_1_2p5G_VALUE   ( 0x0 )
#define SDMA0_RR_WEIGHT_TX_BBH_ID_XLMAC1_1_RGMII_VALUE  ( 0x0 )
#define SDMA0_RR_WEIGHT_TX_BBH_ID_XLMAC1_2_RGMII_VALUE  ( 0x0 )
#define SDMA0_RR_WEIGHT_TX_BBH_ID_PON_VALUE             ( 0x0 )
#define SDMA0_RR_WEIGHT_TX_BBH_ID_DSL_VALUE             ( 0x0 )
#define SDMA1_RR_WEIGHT_TX_BBH_ID_XLMAC0_0_10G_VALUE    ( 0x0 )
#define SDMA1_RR_WEIGHT_TX_BBH_ID_XLMAC0_2_1G_VALUE     ( 0x0 )
#define SDMA1_RR_WEIGHT_TX_BBH_ID_XLMAC0_3_1G_VALUE     ( 0x0 )
#define SDMA1_RR_WEIGHT_TX_BBH_ID_XLMAC1_0_RGMII_VALUE  ( 0x0 )

#define DMA_MAX_READ_ON_THE_FLY ( 16 )
#define SDMA_MAX_READ_ON_THE_FLY ( 8 )

typedef struct
{
    uint8_t into_urgent_threshold;
    uint8_t out_of_urgent_threshold;
} urgent_threhold_t;

typedef struct
{
    uint8_t rx_side;
    uint8_t tx_side;
} strict_priority_t;

typedef struct
{
    uint8_t rx_side;
    uint8_t tx_side;
} rr_weight_t;

typedef struct
{
    uint8_t rx_side;
    uint8_t tx_side;
} bb_source_t;

#ifdef USE_BDMF_SHELL
void drv_dma_cli_init(bdmfmon_handle_t driver_dir);
void drv_dma_cli_exit(bdmfmon_handle_t driver_dir);
#endif

#ifdef __cplusplus
}
#endif

#endif
