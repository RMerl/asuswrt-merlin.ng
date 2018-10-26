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

/******************************************************************************/
/*                                                                            */
/* File Description:                                                          */
/*                                                                            */
/* This file contains the implementation of the Lilac BBH driver              */
/*                                                                            */
/******************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/

#include "rdp_subsystem_common.h"
#include "rdp_drv_bbh.h"


#ifdef  FSSIM
#define DEBUG_TRACE
#define DEBUG_TRAafter_init_ih
#define DEBUG_TRAwith_gpon
#define DEBUG_TRAwith_gbe
#else
#define DEBUG_DELAY() mdelay( 1 )

#define DEBUG_PRINT()

#define DEBUG_TRACE                 DEBUG_PRINT() DEBUG_DELAY()
#define DEBUG_TRAafter_init_ih   DEBUG_PRINT() DEBUG_DELAY()
#define DEBUG_TRAwith_gpon       DEBUG_PRINT() DEBUG_DELAY()
#define DEBUG_TRAwith_gbe        DEBUG_PRINT() DEBUG_DELAY()

#endif /* FSSIM */


/******************************************************************************/
/*                                                                            */
/* Types and values definitions                                               */
/*                                                                            */
/******************************************************************************/

#define CS_NUMBER_OF_BITS_IN_REGISTER                                     ( 32 )

#define CS_RX_MAXIMAL_FLOW_INDEX_FOR_MINPKTSEL0_AND_MAXPKTSEL0_REGISTERS  ( 15 )

#define CS_RX_FIELD_LENGTH_FOR_MINPKTSEL_AND_MAXPKTSEL_REGISTERS          ( 2 )

#define CS_RX_MAXIMAL_FLOW_INDEX_FOR_IHCLASS0_REGISTER                    ( 7 )
#define CS_RX_MAXIMAL_FLOW_INDEX_FOR_IHCLASS1_REGISTER                    ( 15 )
#define CS_RX_MAXIMAL_FLOW_INDEX_FOR_IHCLASS2_REGISTER                    ( 23 )

#define CS_RX_FIELD_LENGTH_FOR_IHCLASS_REGISTERS                          ( 4 )

/* ingress buffer size */
#define CS_DRV_BBH_RX_INGRESS_BUFFER_SIZE                              ( 128 )

/* DDR buffer size */
#define CS_DRV_BBH_RX_DDR_BUFFER_SIZE                                  ( 4096 )

/* The max Tcont number in the GMP */
#define CS_DRV_MAX_NUM_OF_TCONTS_SW									( 32 )
/* The max physical Tcont id number */
#define CS_DRV_MAX_NUM_OF_TCONTS_PHYSICAL							( 39 )

/* TODO: get the proper value from VLSI */
#define BBH_RX_DOCSIS_BYOI_BB_SPLIT (0x0)

/******************************************************************************/
/*                                                                            */
/* Macros definitions                                                         */
/*                                                                            */
/******************************************************************************/

/* gets a sequence of bits out of a given value (value), according to a given length (length)
   and ls_bit_number */
#define MS_GET_BITS( value , ls_bit_number, length )			( ( ( value ) >> ( ls_bit_number ) ) & ( ( 1 << ( length ) ) - 1 ) )

/* writes a sequence of bits (write_value) with a given length (length) to a given
   value (value). the offset is according to ls_bit_number */
#define MS_SET_BITS( value , ls_bit_number , length , write_value )  ( ( value ) &= ~ ( ( ( 1 << ( length ) ) - 1 ) << ( ls_bit_number ) ) , ( value ) |= ( ( write_value ) << ( ls_bit_number ) ) )


/******************************************************************************/
/*                                                                            */
/* Global variables definitions                                               */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/* Functions prototypes                                                       */
/*                                                                            */
/******************************************************************************/

uint8_t f_convert_tx_pd_fifo_size_from_user_to_hw_format ( uint8_t xi_pd_fifo_size ) ;
uint8_t f_convert_tx_pd_fifo_size_from_hw_to_user_format ( uint8_t xi_pd_fifo_size ) ;

int32_t f_minimum_packet_size_is_valid ( uint8_t xi_minimum_packet_size ,
                                                 uint8_t xi_packet_header_offset ) ;
int32_t f_maximum_packet_size_is_valid ( uint16_t xi_maximum_packet_size ,
                                                 uint8_t xi_packet_header_offset ,
                                                 uint8_t xi_reassembly_offset_in_8_byte ) ;



/******************************************************************************/
/*                                                                            */
/* API functions implementations                                              */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bbh_tx_set_configuration                                       */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BBH Driver - TX Set configuration                                        */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets configuration of the TX part of BBH block.            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_port_index - Port index                                               */
/*                                                                            */
/*   xi_bbh_tx_configuration - BBH TX configuration.                          */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_BBH_ERROR - Return code                                       */
/*     DRV_BBH_NO_ERROR - No error                                      */
/*     DRV_BBH_INVALID_PORT_INDEX - Invalid port index                  */
/*                                                                            */
/******************************************************************************/
DRV_BBH_ERROR fi_bl_drv_bbh_tx_set_configuration ( DRV_BBH_PORT_INDEX xi_port_index ,
                                                          const DRV_BBH_TX_CONFIGURATION * xi_bbh_tx_configuration )
{

    BBH_TX_CONFIGURATIONS_BBCFG_TX tx_bbcfg_tx ;
    BBH_TX_CONFIGURATIONS_BBCFG1_TX tx_bbcfg1_tx ;
    BBH_TX_CONFIGURATIONS_DDRCFG_TX tx_ddrcfg_tx ;
    BBH_TX_CONFIGURATIONS_DDRHNBASE tx_hnbase ;
    BBH_TX_CONFIGURATIONS_TASKLSB tx_tasklsb ;
    BBH_TX_CONFIGURATIONS_TASKMSB tx_taskmsb ;
    BBH_TX_CONFIGURATIONS_TASK8_39 tx_task8_39 ;
    BBH_TX_CONFIGURATIONS_PDSIZE0_7 tx_pdsize0_7 ;
    BBH_TX_CONFIGURATIONS_PDSIZE8_39 tx_pdsize8_39 ;
    BBH_TX_CONFIGURATIONS_PDBASE0_3 tx_pdbase0_3 ;
    BBH_TX_CONFIGURATIONS_PDBASE4_7 tx_pdbase4_7 ;
    BBH_TX_CONFIGURATIONS_PDBASE8_39 tx_pdbase8_39 ;
    BBH_TX_CONFIGURATIONS_PD_BYTE_TH_EN tx_pd_byte_th_en ;
    BBH_TX_CONFIGURATIONS_PD_BYTE_TH tx_pd_byte_th ;
    BBH_TX_CONFIGURATIONS_DMACFG_TX tx_dmacfg_tx ;
    BBH_TX_CONFIGURATIONS_SDMACFG_TX tx_sdmacfg_tx ;
    BBH_TX_CONFIGURATIONS_RUNNERCFG tx_runnercfg ;
    BBH_TX_CONFIGURATIONS_MDUMODE tx_mdumode ;
    BBH_TX_CONFIGURATIONS_DDRTMBASE tx_ddrtmbase ;
    BBH_TX_CONFIGURATIONS_EMAC1588 tx_emac1588 ;
    /* New BBH Configuration for DOCSIS */
    BBH_TX_CONFIGURATIONS_MISC_CFG tx_misc_cfg;
    BBH_TX_CONFIGURATIONS_DDR2TMBASE tx_ddr2tmbase;
    BBH_TX_CONFIGURATIONS_DDR2HNBASE tx_ddr2hnbase;

    if ( xi_port_index >= DRV_BBH_NUMBER_OF_PORTS )

    {
        return ( DRV_BBH_INVALID_PORT_INDEX ) ;
    }

    DEBUG_TRAafter_init_ih ;

    if ( ( xi_bbh_tx_configuration->multicast_header_size < DRV_BBH_MINIMAL_MULTICAST_HEADER_SIZE ) ||
         ( xi_bbh_tx_configuration->multicast_header_size > DRV_BBH_MAXIMAL_MULTICAST_HEADER_SIZE ) )
    {
        return ( DRV_BBH_INVALID_MULTICAST_HEADER_SIZE ) ;
    }

    DEBUG_TRAafter_init_ih ;

    if ( ( xi_bbh_tx_configuration->pd_fifo_size_0 < DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE ) ||
         ( xi_bbh_tx_configuration->pd_fifo_size_0 > DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE ) ||
         ( xi_bbh_tx_configuration->pd_fifo_size_1 < DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE ) ||
         ( xi_bbh_tx_configuration->pd_fifo_size_1 > DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE ) ||
         ( xi_bbh_tx_configuration->pd_fifo_size_2 < DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE ) ||
         ( xi_bbh_tx_configuration->pd_fifo_size_2 > DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE ) ||
         ( xi_bbh_tx_configuration->pd_fifo_size_3 < DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE ) ||
         ( xi_bbh_tx_configuration->pd_fifo_size_3 > DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE ) ||
         ( xi_bbh_tx_configuration->pd_fifo_size_4 < DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE ) ||
         ( xi_bbh_tx_configuration->pd_fifo_size_4 > DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE ) ||
         ( xi_bbh_tx_configuration->pd_fifo_size_5 < DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE ) ||
         ( xi_bbh_tx_configuration->pd_fifo_size_5 > DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE ) ||
         ( xi_bbh_tx_configuration->pd_fifo_size_6 < DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE ) ||
         ( xi_bbh_tx_configuration->pd_fifo_size_6 > DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE ) ||
         ( xi_bbh_tx_configuration->pd_fifo_size_7 < DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE ) ||
         ( xi_bbh_tx_configuration->pd_fifo_size_7 > DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE ) ||
         ( xi_bbh_tx_configuration->pd_fifo_size_8_15 < DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE ) ||
         ( xi_bbh_tx_configuration->pd_fifo_size_8_15 > DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE ) ||
         ( xi_bbh_tx_configuration->pd_fifo_size_16_23 < DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE ) ||
         ( xi_bbh_tx_configuration->pd_fifo_size_16_23 > DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE ) ||
         ( xi_bbh_tx_configuration->pd_fifo_size_24_31 < DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE ) ||
         ( xi_bbh_tx_configuration->pd_fifo_size_24_31 > DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE ) ||
         ( xi_bbh_tx_configuration->pd_fifo_size_32_39 < DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE ) ||
         ( xi_bbh_tx_configuration->pd_fifo_size_32_39 > DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE ) )
    {
        return ( DRV_BBH_INVALID_TX_PD_FIFO_SIZE ) ;
    }

    DEBUG_TRAafter_init_ih ;

    if ( ( xi_bbh_tx_configuration->pd_prefetch_byte_threshold_0_in_32_byte > DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE ) ||
         ( xi_bbh_tx_configuration->pd_prefetch_byte_threshold_1_in_32_byte > DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE ) ||
         ( xi_bbh_tx_configuration->pd_prefetch_byte_threshold_2_in_32_byte > DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE ) ||
         ( xi_bbh_tx_configuration->pd_prefetch_byte_threshold_3_in_32_byte > DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE ) ||
         ( xi_bbh_tx_configuration->pd_prefetch_byte_threshold_4_in_32_byte > DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE ) ||
         ( xi_bbh_tx_configuration->pd_prefetch_byte_threshold_5_in_32_byte > DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE ) ||
         ( xi_bbh_tx_configuration->pd_prefetch_byte_threshold_6_in_32_byte > DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE ) ||
         ( xi_bbh_tx_configuration->pd_prefetch_byte_threshold_7_in_32_byte > DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE ) ||
         ( xi_bbh_tx_configuration->pd_prefetch_byte_threshold_8_39_in_32_byte > DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE ) )
    {
        return ( DRV_BBH_INVALID_PD_PREFETCH_BYTE_THRESHOLD ) ;
    }

    DEBUG_TRAafter_init_ih ;

    if ( ( xi_bbh_tx_configuration->dma_read_requests_maximal_number < DRV_BBH_TX_MINIMAL_VALUE_FOR_DMA_READ_REQUESTS_MAXIMAL_NUMBER ) ||
         ( xi_bbh_tx_configuration->dma_read_requests_maximal_number > DRV_BBH_TX_MAXIMAL_VALUE_FOR_DMA_READ_REQUESTS_MAXIMAL_NUMBER ) )
    {
        return ( DRV_BBH_INVALID_DMA_READ_REQUESTS_MAXIMAL_NUMBER ) ;
    }

    DEBUG_TRAafter_init_ih ;

    if ( ( xi_bbh_tx_configuration->sdma_read_requests_maximal_number < DRV_BBH_TX_MINIMAL_VALUE_FOR_SDMA_READ_REQUESTS_MAXIMAL_NUMBER ) ||
         ( xi_bbh_tx_configuration->sdma_read_requests_maximal_number > DRV_BBH_TX_MAXIMAL_VALUE_FOR_SDMA_READ_REQUESTS_MAXIMAL_NUMBER ) )
    {
        return ( DRV_BBH_INVALID_SDMA_READ_REQUESTS_MAXIMAL_NUMBER ) ;
    }

    DEBUG_TRAafter_init_ih ;


    BBH_TX_CONFIGURATIONS_BBCFG_TX_READ( xi_port_index , tx_bbcfg_tx ) ;
    tx_bbcfg_tx.dmaroute = xi_bbh_tx_configuration->dma_route_address ;
    tx_bbcfg_tx.runnerroute = xi_bbh_tx_configuration->runner_route_address ;
    tx_bbcfg_tx.bpmroute = xi_bbh_tx_configuration->bpm_route_address ;
    BBH_TX_CONFIGURATIONS_BBCFG_TX_WRITE( xi_port_index , tx_bbcfg_tx ) ;

    DEBUG_TRAafter_init_ih ;

    BBH_TX_CONFIGURATIONS_BBCFG1_TX_READ( xi_port_index , tx_bbcfg1_tx ) ;
    tx_bbcfg1_tx.sdmaroute = xi_bbh_tx_configuration->sdma_route_address;
    tx_bbcfg1_tx.sbpmroute = xi_bbh_tx_configuration->sbpm_route_address;
    tx_bbcfg1_tx.rnrstsroute = xi_bbh_tx_configuration->runner_sts_route ;
    BBH_TX_CONFIGURATIONS_BBCFG1_TX_WRITE( xi_port_index , tx_bbcfg1_tx ) ;

    DEBUG_TRAafter_init_ih ;

    BBH_TX_CONFIGURATIONS_DDRCFG_TX_READ( xi_port_index , tx_ddrcfg_tx ) ;
    tx_ddrcfg_tx.bufsize = xi_bbh_tx_configuration->ddr_buffer_size ;
    tx_ddrcfg_tx.byteresul = xi_bbh_tx_configuration->payload_offset_resolution ;
    tx_ddrcfg_tx.hnsize = xi_bbh_tx_configuration->multicast_header_size ;
    BBH_TX_CONFIGURATIONS_DDRCFG_TX_WRITE( xi_port_index , tx_ddrcfg_tx ) ;

    DEBUG_TRAafter_init_ih ;

    BBH_TX_CONFIGURATIONS_DDRHNBASE_READ( xi_port_index , tx_hnbase ) ;
    tx_hnbase.hnbase = xi_bbh_tx_configuration->ddr1_multicast_headers_base_address_in_byte ;
    BBH_TX_CONFIGURATIONS_DDRHNBASE_WRITE( xi_port_index , tx_hnbase ) ;

    BBH_TX_CONFIGURATIONS_DDR2HNBASE_READ( xi_port_index , tx_ddr2hnbase ) ;
    tx_ddr2hnbase.hnbase = xi_bbh_tx_configuration->ddr2_multicast_headers_base_address_in_byte;
    BBH_TX_CONFIGURATIONS_DDR2HNBASE_WRITE( xi_port_index , tx_ddr2hnbase ) ;

    DEBUG_TRAafter_init_ih ;

    BBH_TX_CONFIGURATIONS_TASKLSB_READ( xi_port_index , tx_tasklsb ) ;
    tx_tasklsb.task0 = xi_bbh_tx_configuration->task_0 ;
    tx_tasklsb.task1 = xi_bbh_tx_configuration->task_1 ;
    tx_tasklsb.task2 = xi_bbh_tx_configuration->task_2 ;
    tx_tasklsb.task3 = xi_bbh_tx_configuration->task_3 ;
    BBH_TX_CONFIGURATIONS_TASKLSB_WRITE( xi_port_index , tx_tasklsb ) ;

    DEBUG_TRAafter_init_ih ;

    BBH_TX_CONFIGURATIONS_TASKMSB_READ( xi_port_index , tx_taskmsb ) ;
    tx_taskmsb.task4 = xi_bbh_tx_configuration->task_4 ;
    tx_taskmsb.task5 = xi_bbh_tx_configuration->task_5 ;
    tx_taskmsb.task6 = xi_bbh_tx_configuration->task_6 ;
    tx_taskmsb.task7 = xi_bbh_tx_configuration->task_7 ;
    BBH_TX_CONFIGURATIONS_TASKMSB_WRITE( xi_port_index , tx_taskmsb ) ;

    DEBUG_TRAafter_init_ih ;

    BBH_TX_CONFIGURATIONS_TASK8_39_READ( xi_port_index , tx_task8_39 ) ;
    tx_task8_39.task8_39 = xi_bbh_tx_configuration->task_8_39 ;
    BBH_TX_CONFIGURATIONS_TASK8_39_WRITE( xi_port_index , tx_task8_39 ) ;

    DEBUG_TRAafter_init_ih ;

    BBH_TX_CONFIGURATIONS_PDSIZE0_7_READ( xi_port_index , tx_pdsize0_7 ) ;
    tx_pdsize0_7.fifosize0 = f_convert_tx_pd_fifo_size_from_user_to_hw_format ( xi_bbh_tx_configuration->pd_fifo_size_0 ) ;
    tx_pdsize0_7.fifosize1 = f_convert_tx_pd_fifo_size_from_user_to_hw_format ( xi_bbh_tx_configuration->pd_fifo_size_1 ) ;
    tx_pdsize0_7.fifosize2 = f_convert_tx_pd_fifo_size_from_user_to_hw_format ( xi_bbh_tx_configuration->pd_fifo_size_2 ) ;
    tx_pdsize0_7.fifosize3 = f_convert_tx_pd_fifo_size_from_user_to_hw_format ( xi_bbh_tx_configuration->pd_fifo_size_3 ) ;
    tx_pdsize0_7.fifosize4 = f_convert_tx_pd_fifo_size_from_user_to_hw_format ( xi_bbh_tx_configuration->pd_fifo_size_4 ) ;
    tx_pdsize0_7.fifosize5 = f_convert_tx_pd_fifo_size_from_user_to_hw_format ( xi_bbh_tx_configuration->pd_fifo_size_5 ) ;
    tx_pdsize0_7.fifosize6 = f_convert_tx_pd_fifo_size_from_user_to_hw_format ( xi_bbh_tx_configuration->pd_fifo_size_6 ) ;
    tx_pdsize0_7.fifosize7 = f_convert_tx_pd_fifo_size_from_user_to_hw_format ( xi_bbh_tx_configuration->pd_fifo_size_7 ) ;
    BBH_TX_CONFIGURATIONS_PDSIZE0_7_WRITE( xi_port_index , tx_pdsize0_7 ) ;

    DEBUG_TRAafter_init_ih ;

    BBH_TX_CONFIGURATIONS_PDSIZE8_39_READ( xi_port_index , tx_pdsize8_39 ) ;
    tx_pdsize8_39.fifosize8_15 = f_convert_tx_pd_fifo_size_from_user_to_hw_format ( xi_bbh_tx_configuration->pd_fifo_size_8_15 ) ;
    tx_pdsize8_39.fifosize16_23 = f_convert_tx_pd_fifo_size_from_user_to_hw_format ( xi_bbh_tx_configuration->pd_fifo_size_16_23 ) ;
    tx_pdsize8_39.fifosize24_31 = f_convert_tx_pd_fifo_size_from_user_to_hw_format ( xi_bbh_tx_configuration->pd_fifo_size_24_31 ) ;
    tx_pdsize8_39.fifosize32_39 = f_convert_tx_pd_fifo_size_from_user_to_hw_format ( xi_bbh_tx_configuration->pd_fifo_size_32_39 ) ;
    BBH_TX_CONFIGURATIONS_PDSIZE8_39_WRITE( xi_port_index , tx_pdsize8_39 ) ;

    DEBUG_TRAafter_init_ih ;

    BBH_TX_CONFIGURATIONS_PDBASE0_3_READ( xi_port_index , tx_pdbase0_3 ) ;
    tx_pdbase0_3.fifobase0 = xi_bbh_tx_configuration->pd_fifo_base_0 ;
    tx_pdbase0_3.fifobase1 = xi_bbh_tx_configuration->pd_fifo_base_1 ;
    tx_pdbase0_3.fifobase2 = xi_bbh_tx_configuration->pd_fifo_base_2 ;
    tx_pdbase0_3.fifobase3 = xi_bbh_tx_configuration->pd_fifo_base_3 ;
    BBH_TX_CONFIGURATIONS_PDBASE0_3_WRITE( xi_port_index , tx_pdbase0_3 ) ;

    DEBUG_TRAafter_init_ih ;

    BBH_TX_CONFIGURATIONS_PDBASE4_7_READ( xi_port_index , tx_pdbase4_7 ) ;
    tx_pdbase4_7.fifobase4 = xi_bbh_tx_configuration->pd_fifo_base_4 ;
    tx_pdbase4_7.fifobase5 = xi_bbh_tx_configuration->pd_fifo_base_5 ;
    tx_pdbase4_7.fifobase6 = xi_bbh_tx_configuration->pd_fifo_base_6 ;
    tx_pdbase4_7.fifobase7 = xi_bbh_tx_configuration->pd_fifo_base_7 ;
    BBH_TX_CONFIGURATIONS_PDBASE4_7_WRITE( xi_port_index , tx_pdbase4_7 ) ;

    DEBUG_TRAafter_init_ih ;

    BBH_TX_CONFIGURATIONS_PDBASE8_39_READ( xi_port_index , tx_pdbase8_39 ) ;
    tx_pdbase8_39.fifobase8_15 = xi_bbh_tx_configuration->pd_fifo_base_8_15 ;
    tx_pdbase8_39.fifobase16_23 = xi_bbh_tx_configuration->pd_fifo_base_16_23 ;
    tx_pdbase8_39.fifobase24_31 = xi_bbh_tx_configuration->pd_fifo_base_24_31 ;
    tx_pdbase8_39.fifobase32_39 = xi_bbh_tx_configuration->pd_fifo_base_32_39 ;
    BBH_TX_CONFIGURATIONS_PDBASE8_39_WRITE( xi_port_index , tx_pdbase8_39 ) ;

    DEBUG_TRAafter_init_ih ;

    BBH_TX_CONFIGURATIONS_PD_BYTE_TH_EN_READ( xi_port_index , tx_pd_byte_th_en ) ;
    tx_pd_byte_th_en.pdlimiten = xi_bbh_tx_configuration->pd_prefetch_byte_threshold_enable ;
    BBH_TX_CONFIGURATIONS_PD_BYTE_TH_EN_WRITE( xi_port_index , tx_pd_byte_th_en ) ;

    DEBUG_TRAafter_init_ih ;

    BBH_TX_CONFIGURATIONS_PD_BYTE_TH_READ( xi_port_index , 0 , tx_pd_byte_th ) ;
    tx_pd_byte_th.pdlimiteven = xi_bbh_tx_configuration->pd_prefetch_byte_threshold_0_in_32_byte ;
    tx_pd_byte_th.pdlimitodd = xi_bbh_tx_configuration->pd_prefetch_byte_threshold_1_in_32_byte ;
    BBH_TX_CONFIGURATIONS_PD_BYTE_TH_WRITE( xi_port_index , 0 , tx_pd_byte_th ) ;

    DEBUG_TRAafter_init_ih ;

    BBH_TX_CONFIGURATIONS_PD_BYTE_TH_READ( xi_port_index , 1 , tx_pd_byte_th ) ;
    tx_pd_byte_th.pdlimiteven = xi_bbh_tx_configuration->pd_prefetch_byte_threshold_2_in_32_byte ;
    tx_pd_byte_th.pdlimitodd = xi_bbh_tx_configuration->pd_prefetch_byte_threshold_3_in_32_byte ;
    BBH_TX_CONFIGURATIONS_PD_BYTE_TH_WRITE( xi_port_index , 1 , tx_pd_byte_th ) ;

    DEBUG_TRAafter_init_ih ;

    BBH_TX_CONFIGURATIONS_PD_BYTE_TH_READ( xi_port_index , 2 , tx_pd_byte_th ) ;
    tx_pd_byte_th.pdlimiteven = xi_bbh_tx_configuration->pd_prefetch_byte_threshold_4_in_32_byte ;
    tx_pd_byte_th.pdlimitodd = xi_bbh_tx_configuration->pd_prefetch_byte_threshold_5_in_32_byte ;
    BBH_TX_CONFIGURATIONS_PD_BYTE_TH_WRITE( xi_port_index , 2 , tx_pd_byte_th ) ;

    DEBUG_TRAafter_init_ih ;

    BBH_TX_CONFIGURATIONS_PD_BYTE_TH_READ( xi_port_index , 3 , tx_pd_byte_th ) ;
    tx_pd_byte_th.pdlimiteven = xi_bbh_tx_configuration->pd_prefetch_byte_threshold_6_in_32_byte ;
    tx_pd_byte_th.pdlimitodd = xi_bbh_tx_configuration->pd_prefetch_byte_threshold_7_in_32_byte ;
    BBH_TX_CONFIGURATIONS_PD_BYTE_TH_WRITE( xi_port_index , 3 , tx_pd_byte_th ) ;

    DEBUG_TRAafter_init_ih ;

    /* TCONTs 8-39 have a common configuration */
    BBH_TX_CONFIGURATIONS_PD_BYTE_TH_READ( xi_port_index , 4 , tx_pd_byte_th ) ;
    tx_pd_byte_th.pdlimiteven = xi_bbh_tx_configuration->pd_prefetch_byte_threshold_8_39_in_32_byte ;
    BBH_TX_CONFIGURATIONS_PD_BYTE_TH_WRITE( xi_port_index , 4 , tx_pd_byte_th ) ;

    DEBUG_TRAafter_init_ih ;

    BBH_TX_CONFIGURATIONS_DMACFG_TX_READ( xi_port_index , tx_dmacfg_tx ) ;
    tx_dmacfg_tx.descbase = xi_bbh_tx_configuration->dma_read_requests_fifo_base_address ;
    tx_dmacfg_tx.maxreq = xi_bbh_tx_configuration->dma_read_requests_maximal_number ;
    BBH_TX_CONFIGURATIONS_DMACFG_TX_WRITE( xi_port_index , tx_dmacfg_tx ) ;

    DEBUG_TRAafter_init_ih ;

    BBH_TX_CONFIGURATIONS_SDMACFG_TX_READ( xi_port_index , tx_sdmacfg_tx ) ;
    tx_sdmacfg_tx.descbase = xi_bbh_tx_configuration->sdma_read_requests_fifo_base_address ;
    tx_sdmacfg_tx.maxreq = xi_bbh_tx_configuration->sdma_read_requests_maximal_number ;
    BBH_TX_CONFIGURATIONS_SDMACFG_TX_WRITE( xi_port_index , tx_sdmacfg_tx ) ;

    DEBUG_TRAafter_init_ih ;

    BBH_TX_CONFIGURATIONS_RUNNERCFG_READ( xi_port_index , tx_runnercfg ) ;
    tx_runnercfg.tcontaddr = xi_bbh_tx_configuration->tcont_address_in_8_byte ;
    tx_runnercfg.skbaddr = xi_bbh_tx_configuration->skb_address ;
    BBH_TX_CONFIGURATIONS_RUNNERCFG_WRITE( xi_port_index , tx_runnercfg ) ;

    DEBUG_TRAafter_init_ih ;

    BBH_TX_CONFIGURATIONS_MDUMODE_READ( xi_port_index , tx_mdumode ) ;
    tx_mdumode.mduen = xi_bbh_tx_configuration->mdu_mode_enable ;
    tx_mdumode.ptraddr = xi_bbh_tx_configuration->mdu_mode_read_pointer_address_in_8_byte ;
    BBH_TX_CONFIGURATIONS_MDUMODE_WRITE( xi_port_index , tx_mdumode ) ;

    DEBUG_TRAafter_init_ih ;

    BBH_TX_CONFIGURATIONS_DDRTMBASE_READ( xi_port_index , tx_ddrtmbase ) ;
    tx_ddrtmbase.ddrtmbase = xi_bbh_tx_configuration->ddr1_tm_base_address ;
    BBH_TX_CONFIGURATIONS_DDRTMBASE_WRITE( xi_port_index , tx_ddrtmbase ) ;

    BBH_TX_CONFIGURATIONS_DDR2TMBASE_READ( xi_port_index , tx_ddr2tmbase ) ;
	tx_ddr2tmbase.ddrtmbase = xi_bbh_tx_configuration->ddr2_tm_base_address ;
	BBH_TX_CONFIGURATIONS_DDR2TMBASE_WRITE( xi_port_index , tx_ddr2tmbase ) ;

    DEBUG_TRAafter_init_ih ;

    BBH_TX_CONFIGURATIONS_EMAC1588_READ( xi_port_index , tx_emac1588 ) ;
    tx_emac1588.emac1588 = xi_bbh_tx_configuration->emac_1588_enable ;
    BBH_TX_CONFIGURATIONS_EMAC1588_WRITE( xi_port_index , tx_emac1588 ) ;

    DEBUG_TRAafter_init_ih ;

    BBH_TX_CONFIGURATIONS_MISC_CFG_READ(xi_port_index, tx_misc_cfg);
    tx_misc_cfg.byoi_no_fpm = xi_bbh_tx_configuration->byoi_no_fpm_release;
    tx_misc_cfg.byoi_direct = xi_bbh_tx_configuration->byoi_direct;
    tx_misc_cfg.buf_size_sel = xi_bbh_tx_configuration->fpm_buff_size_set;
    BBH_TX_CONFIGURATIONS_MISC_CFG_WRITE(xi_port_index, tx_misc_cfg);

    return ( DRV_BBH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_bbh_tx_set_configuration ) ;

#ifndef _CFE_
/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bbh_tx_get_configuration                                       */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BBH Driver - TX Get configuration                                        */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the configuration of the TX part of BBH block.        */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_port_index - Port index                                               */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_bbh_tx_configuration - BBH TX configuration.                          */
/*                                                                            */
/*   DRV_BBH_ERROR - Return code                                       */
/*     DRV_BBH_NO_ERROR - No error                                      */
/*     DRV_BBH_INVALID_PORT_INDEX - Invalid port index                  */
/*                                                                            */
/******************************************************************************/
DRV_BBH_ERROR fi_bl_drv_bbh_tx_get_configuration ( DRV_BBH_PORT_INDEX xi_port_index ,
                                                          DRV_BBH_TX_CONFIGURATION * const xo_bbh_tx_configuration )
{
    BBH_TX_CONFIGURATIONS_BBCFG_TX tx_bbcfg_tx ;
    BBH_TX_CONFIGURATIONS_BBCFG1_TX tx_bbcfg1_tx ;
    BBH_TX_CONFIGURATIONS_DDRCFG_TX tx_ddrcfg_tx ;
    BBH_TX_CONFIGURATIONS_DDRHNBASE tx_hnbase ;
    BBH_TX_CONFIGURATIONS_TASKLSB tx_tasklsb ;
    BBH_TX_CONFIGURATIONS_TASKMSB tx_taskmsb ;
    BBH_TX_CONFIGURATIONS_TASK8_39 tx_task8_39 ;
    BBH_TX_CONFIGURATIONS_PDSIZE0_7 tx_pdsize0_7 ;
    BBH_TX_CONFIGURATIONS_PDSIZE8_39 tx_pdsize8_39 ;
    BBH_TX_CONFIGURATIONS_PDBASE0_3 tx_pdbase0_3 ;
    BBH_TX_CONFIGURATIONS_PDBASE4_7 tx_pdbase4_7 ;
    BBH_TX_CONFIGURATIONS_PDBASE8_39 tx_pdbase8_39 ;
    BBH_TX_CONFIGURATIONS_PD_BYTE_TH_EN tx_pd_byte_th_en ;
    BBH_TX_CONFIGURATIONS_PD_BYTE_TH tx_pd_byte_th ;
    BBH_TX_CONFIGURATIONS_DMACFG_TX tx_dmacfg_tx ;
    BBH_TX_CONFIGURATIONS_SDMACFG_TX tx_sdmacfg_tx ;
    BBH_TX_CONFIGURATIONS_RUNNERCFG tx_runnercfg ;
    BBH_TX_CONFIGURATIONS_MDUMODE tx_mdumode ;
    BBH_TX_CONFIGURATIONS_DDRTMBASE tx_ddrtmbase ;
    BBH_TX_CONFIGURATIONS_EMAC1588 tx_emac1588 ;
    /* New BBH Configuration for DOCSIS */
    BBH_TX_CONFIGURATIONS_MISC_CFG tx_misc_cfg;
    BBH_TX_CONFIGURATIONS_DDR2TMBASE tx_ddr2tmbase;
    BBH_TX_CONFIGURATIONS_DDR2HNBASE tx_hn2base;

    if ( xi_port_index >= DRV_BBH_NUMBER_OF_PORTS )

    {
        return ( DRV_BBH_INVALID_PORT_INDEX ) ;
    }


    BBH_TX_CONFIGURATIONS_BBCFG_TX_READ( xi_port_index , tx_bbcfg_tx ) ;
    xo_bbh_tx_configuration->dma_route_address = tx_bbcfg_tx.dmaroute ;
    xo_bbh_tx_configuration->runner_route_address = tx_bbcfg_tx.runnerroute ;
    xo_bbh_tx_configuration->bpm_route_address = tx_bbcfg_tx.bpmroute ;

    BBH_TX_CONFIGURATIONS_BBCFG1_TX_READ( xi_port_index , tx_bbcfg1_tx ) ;
    xo_bbh_tx_configuration->sdma_route_address = tx_bbcfg1_tx.sdmaroute ;
    xo_bbh_tx_configuration->sbpm_route_address = tx_bbcfg1_tx.sbpmroute ;

    BBH_TX_CONFIGURATIONS_DDRCFG_TX_READ( xi_port_index , tx_ddrcfg_tx ) ;
    xo_bbh_tx_configuration->ddr_buffer_size = tx_ddrcfg_tx.bufsize ;
    xo_bbh_tx_configuration->payload_offset_resolution = tx_ddrcfg_tx.byteresul ;
    xo_bbh_tx_configuration->multicast_header_size = tx_ddrcfg_tx.hnsize ;

    BBH_TX_CONFIGURATIONS_DDRHNBASE_READ( xi_port_index , tx_hnbase ) ;
    xo_bbh_tx_configuration->ddr1_multicast_headers_base_address_in_byte = tx_hnbase.hnbase ;

    BBH_TX_CONFIGURATIONS_DDRHNBASE_READ( xi_port_index , tx_hn2base ) ;
    xo_bbh_tx_configuration->ddr2_multicast_headers_base_address_in_byte = tx_hn2base.hnbase ;

    BBH_TX_CONFIGURATIONS_TASKLSB_READ( xi_port_index , tx_tasklsb ) ;
    xo_bbh_tx_configuration->task_0 = tx_tasklsb.task0 ;
    xo_bbh_tx_configuration->task_1 = tx_tasklsb.task1 ;
    xo_bbh_tx_configuration->task_2 = tx_tasklsb.task2 ;
    xo_bbh_tx_configuration->task_3 = tx_tasklsb.task3 ;

    BBH_TX_CONFIGURATIONS_TASKMSB_READ( xi_port_index , tx_taskmsb ) ;
    xo_bbh_tx_configuration->task_4 = tx_taskmsb.task4 ;
    xo_bbh_tx_configuration->task_5 = tx_taskmsb.task5 ;
    xo_bbh_tx_configuration->task_6 = tx_taskmsb.task6 ;
    xo_bbh_tx_configuration->task_7 = tx_taskmsb.task7 ;

    BBH_TX_CONFIGURATIONS_TASK8_39_READ( xi_port_index , tx_task8_39 ) ;
    xo_bbh_tx_configuration->task_8_39 = tx_task8_39.task8_39 ;

    BBH_TX_CONFIGURATIONS_PDSIZE0_7_READ( xi_port_index , tx_pdsize0_7 ) ;
    xo_bbh_tx_configuration->pd_fifo_size_0 = f_convert_tx_pd_fifo_size_from_hw_to_user_format ( tx_pdsize0_7.fifosize0 ) ;
    xo_bbh_tx_configuration->pd_fifo_size_1 = f_convert_tx_pd_fifo_size_from_hw_to_user_format ( tx_pdsize0_7.fifosize1 ) ;
    xo_bbh_tx_configuration->pd_fifo_size_2 = f_convert_tx_pd_fifo_size_from_hw_to_user_format ( tx_pdsize0_7.fifosize2 ) ;
    xo_bbh_tx_configuration->pd_fifo_size_3 = f_convert_tx_pd_fifo_size_from_hw_to_user_format ( tx_pdsize0_7.fifosize3 ) ;
    xo_bbh_tx_configuration->pd_fifo_size_4 = f_convert_tx_pd_fifo_size_from_hw_to_user_format ( tx_pdsize0_7.fifosize4 ) ;
    xo_bbh_tx_configuration->pd_fifo_size_5 = f_convert_tx_pd_fifo_size_from_hw_to_user_format ( tx_pdsize0_7.fifosize5 ) ;
    xo_bbh_tx_configuration->pd_fifo_size_6 = f_convert_tx_pd_fifo_size_from_hw_to_user_format ( tx_pdsize0_7.fifosize6 ) ;
    xo_bbh_tx_configuration->pd_fifo_size_7 = f_convert_tx_pd_fifo_size_from_hw_to_user_format ( tx_pdsize0_7.fifosize7 ) ;

    BBH_TX_CONFIGURATIONS_PDSIZE8_39_READ( xi_port_index , tx_pdsize8_39 ) ;
    xo_bbh_tx_configuration->pd_fifo_size_8_15 = f_convert_tx_pd_fifo_size_from_hw_to_user_format ( tx_pdsize8_39.fifosize8_15 ) ;
    xo_bbh_tx_configuration->pd_fifo_size_16_23 = f_convert_tx_pd_fifo_size_from_hw_to_user_format ( tx_pdsize8_39.fifosize16_23 ) ;
    xo_bbh_tx_configuration->pd_fifo_size_24_31 = f_convert_tx_pd_fifo_size_from_hw_to_user_format ( tx_pdsize8_39.fifosize24_31 ) ;
    xo_bbh_tx_configuration->pd_fifo_size_32_39 = f_convert_tx_pd_fifo_size_from_hw_to_user_format ( tx_pdsize8_39.fifosize32_39 ) ;

    BBH_TX_CONFIGURATIONS_PDBASE0_3_READ( xi_port_index , tx_pdbase0_3 ) ;
    xo_bbh_tx_configuration->pd_fifo_base_0 = tx_pdbase0_3.fifobase0 ;
    xo_bbh_tx_configuration->pd_fifo_base_1 = tx_pdbase0_3.fifobase1 ;
    xo_bbh_tx_configuration->pd_fifo_base_2 = tx_pdbase0_3.fifobase2 ;
    xo_bbh_tx_configuration->pd_fifo_base_3 = tx_pdbase0_3.fifobase3 ;

    BBH_TX_CONFIGURATIONS_PDBASE4_7_READ( xi_port_index , tx_pdbase4_7 ) ;
    xo_bbh_tx_configuration->pd_fifo_base_4 = tx_pdbase4_7.fifobase4 ;
    xo_bbh_tx_configuration->pd_fifo_base_5 = tx_pdbase4_7.fifobase5 ;
    xo_bbh_tx_configuration->pd_fifo_base_6 = tx_pdbase4_7.fifobase6 ;
    xo_bbh_tx_configuration->pd_fifo_base_7 = tx_pdbase4_7.fifobase7 ;

    BBH_TX_CONFIGURATIONS_PDBASE8_39_READ( xi_port_index , tx_pdbase8_39 ) ;
    xo_bbh_tx_configuration->pd_fifo_base_8_15 = tx_pdbase8_39.fifobase8_15 ;
    xo_bbh_tx_configuration->pd_fifo_base_16_23 = tx_pdbase8_39.fifobase16_23 ;
    xo_bbh_tx_configuration->pd_fifo_base_24_31 = tx_pdbase8_39.fifobase24_31 ;
    xo_bbh_tx_configuration->pd_fifo_base_32_39 = tx_pdbase8_39.fifobase32_39 ;

    BBH_TX_CONFIGURATIONS_PD_BYTE_TH_EN_READ( xi_port_index , tx_pd_byte_th_en ) ;
    xo_bbh_tx_configuration->pd_prefetch_byte_threshold_enable = tx_pd_byte_th_en.pdlimiten ;

    BBH_TX_CONFIGURATIONS_PD_BYTE_TH_READ( xi_port_index , 0 , tx_pd_byte_th ) ;
    xo_bbh_tx_configuration->pd_prefetch_byte_threshold_0_in_32_byte = tx_pd_byte_th.pdlimiteven ;
    xo_bbh_tx_configuration->pd_prefetch_byte_threshold_1_in_32_byte = tx_pd_byte_th.pdlimitodd ;

    BBH_TX_CONFIGURATIONS_PD_BYTE_TH_READ( xi_port_index , 1 , tx_pd_byte_th ) ;
    xo_bbh_tx_configuration->pd_prefetch_byte_threshold_2_in_32_byte = tx_pd_byte_th.pdlimiteven ;
    xo_bbh_tx_configuration->pd_prefetch_byte_threshold_3_in_32_byte = tx_pd_byte_th.pdlimitodd ;

    BBH_TX_CONFIGURATIONS_PD_BYTE_TH_READ( xi_port_index , 2 , tx_pd_byte_th ) ;
    xo_bbh_tx_configuration->pd_prefetch_byte_threshold_4_in_32_byte = tx_pd_byte_th.pdlimiteven ;
    xo_bbh_tx_configuration->pd_prefetch_byte_threshold_5_in_32_byte = tx_pd_byte_th.pdlimitodd ;

    BBH_TX_CONFIGURATIONS_PD_BYTE_TH_READ( xi_port_index , 3 , tx_pd_byte_th ) ;
    xo_bbh_tx_configuration->pd_prefetch_byte_threshold_6_in_32_byte = tx_pd_byte_th.pdlimiteven ;
    xo_bbh_tx_configuration->pd_prefetch_byte_threshold_7_in_32_byte = tx_pd_byte_th.pdlimitodd ;

    /* TCONTs 8-39 have a common configuration */
    BBH_TX_CONFIGURATIONS_PD_BYTE_TH_READ( xi_port_index , 4 , tx_pd_byte_th ) ;
    xo_bbh_tx_configuration->pd_prefetch_byte_threshold_8_39_in_32_byte = tx_pd_byte_th.pdlimiteven ;

    BBH_TX_CONFIGURATIONS_DMACFG_TX_READ( xi_port_index , tx_dmacfg_tx ) ;
    xo_bbh_tx_configuration->dma_read_requests_fifo_base_address = tx_dmacfg_tx.descbase ;
    xo_bbh_tx_configuration->dma_read_requests_maximal_number = tx_dmacfg_tx.maxreq ;

    BBH_TX_CONFIGURATIONS_SDMACFG_TX_READ( xi_port_index , tx_sdmacfg_tx ) ;
    xo_bbh_tx_configuration->sdma_read_requests_fifo_base_address = tx_sdmacfg_tx.descbase ;
    xo_bbh_tx_configuration->sdma_read_requests_maximal_number = tx_sdmacfg_tx.maxreq ;

    BBH_TX_CONFIGURATIONS_RUNNERCFG_READ( xi_port_index , tx_runnercfg ) ;
    xo_bbh_tx_configuration->tcont_address_in_8_byte = tx_runnercfg.tcontaddr ;
    xo_bbh_tx_configuration->skb_address = tx_runnercfg.skbaddr ;

    BBH_TX_CONFIGURATIONS_MDUMODE_READ( xi_port_index , tx_mdumode ) ;
    xo_bbh_tx_configuration->mdu_mode_enable = tx_mdumode.mduen ;
    xo_bbh_tx_configuration->mdu_mode_read_pointer_address_in_8_byte = tx_mdumode.ptraddr ;

    BBH_TX_CONFIGURATIONS_DDRTMBASE_READ( xi_port_index , tx_ddrtmbase ) ;
    xo_bbh_tx_configuration->ddr1_tm_base_address = tx_ddrtmbase.ddrtmbase ;

    BBH_TX_CONFIGURATIONS_DDR2TMBASE_READ( xi_port_index , tx_ddr2tmbase ) ;
    xo_bbh_tx_configuration->ddr2_tm_base_address = tx_ddr2tmbase.ddrtmbase ;

    BBH_TX_CONFIGURATIONS_EMAC1588_READ( xi_port_index , tx_emac1588 ) ;
    xo_bbh_tx_configuration->emac_1588_enable = tx_emac1588.emac1588 ;

    BBH_TX_CONFIGURATIONS_MISC_CFG_READ( xi_port_index , tx_misc_cfg ) ;
    xo_bbh_tx_configuration->byoi_direct = tx_misc_cfg.byoi_direct;
    xo_bbh_tx_configuration->byoi_no_fpm_release = tx_misc_cfg.byoi_no_fpm;
    xo_bbh_tx_configuration->fpm_buff_size_set = (DRV_BBH_FPM_BUFF_SIZE)tx_misc_cfg.buf_size_sel;

    return ( DRV_BBH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_bbh_tx_get_configuration ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bbh_tx_reset                                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BBH Driver - TX Reset                                                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function enables reset of several internal units of the TX.         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_port_index - Port index                                               */
/*                                                                            */
/*   xi_units_to_reset_bitmask - bitmask of units to reset. Values of the     */
/*     enumeration DRV_BBH_TX_INTERNAL_UNIT should be ORed, as a       */
/*     description of the units to be reset.                                  */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_BBH_ERROR - Return code                                       */
/*     DRV_BBH_NO_ERROR - No error                                      */
/*     DRV_BBH_INVALID_PORT_INDEX - Invalid port index                  */
/*                                                                            */
/******************************************************************************/
DRV_BBH_ERROR fi_bl_drv_bbh_tx_reset ( DRV_BBH_PORT_INDEX xi_port_index ,
                                              uint16_t xi_units_to_reset_bitmask )
{
    /* convert to 32 bit, which is needed for the HAL macro */
    uint32_t units_to_reset_bitmask = xi_units_to_reset_bitmask ;

    if ( xi_port_index >= DRV_BBH_NUMBER_OF_PORTS )

    {
        return ( DRV_BBH_INVALID_PORT_INDEX ) ;
    }

    /* the bitmask is compatible to the register format, so TX_CONFIGURATIONS_TXRSTCMD struct is not used here */
    BBH_TX_CONFIGURATIONS_TXRSTCMD_WRITE( xi_port_index , units_to_reset_bitmask ) ;

    return ( DRV_BBH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_bbh_tx_reset ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bbh_tx_get_counters                                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BBH Driver - TX Get Counters                                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets counters of the TX. Each of these counters is cleared */
/*   when read and freezes when maximum value is reached.                     */
/*   In GPON port, only pd_with_zero_packet_length and tx_packets_from_ddr    */
/*   counters are functional.                                                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_port_index - Port index                                               */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_tx_counters - TX Counters.                                            */
/*                                                                            */
/*   DRV_BBH_ERROR - Return code                                       */
/*     DRV_BBH_NO_ERROR - No error                                      */
/*     DRV_BBH_INVALID_PORT_INDEX - Invalid port index                  */
/*                                                                            */
/******************************************************************************/
DRV_BBH_ERROR fi_bl_drv_bbh_tx_get_counters ( DRV_BBH_PORT_INDEX xi_port_index ,
                                                     DRV_BBH_TX_COUNTERS * const xo_tx_counters )
{
    BBH_TX_DEBUG_SRAMPD tx_srampd ;
    BBH_TX_DEBUG_DDRPD tx_ddrpd ;
    BBH_TX_DEBUG_PDDROP tx_pddrop ;
    BBH_TX_DEBUG_PDEQ0 tx_pdeq0 ;
    BBH_TX_DEBUG_GETNEXTNULL tx_getnextnull ;

    if ( xi_port_index >= DRV_BBH_NUMBER_OF_PORTS )
    {
        return ( DRV_BBH_INVALID_PORT_INDEX ) ;
    }


    BBH_TX_DEBUG_SRAMPD_READ( xi_port_index , tx_srampd ) ;
    xo_tx_counters->tx_packets_from_sram = tx_srampd.srampd ;

    BBH_TX_DEBUG_DDRPD_READ( xi_port_index , tx_ddrpd ) ;
    xo_tx_counters->tx_packets_from_ddr = tx_ddrpd.ddrpd ;

    BBH_TX_DEBUG_PDDROP_READ( xi_port_index , tx_pddrop ) ;
    xo_tx_counters->dropped_pd = tx_pddrop.pddrop ;

    BBH_TX_DEBUG_PDEQ0_READ( xi_port_index , tx_pdeq0 ) ;
    xo_tx_counters->pd_with_zero_packet_length = tx_pdeq0.pdeq0 ;

    BBH_TX_DEBUG_GETNEXTNULL_READ( xi_port_index , tx_getnextnull ) ;
    xo_tx_counters->get_next_null = tx_getnextnull.getnextnull ;

    return ( DRV_BBH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_bbh_tx_get_counters ) ;

#endif
/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bbh_rx_set_configuration                                       */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BBH Driver - RX Set configuration                                        */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets configuration of the RX part of BBH block.            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_port_index - Port index                                               */
/*                                                                            */
/*   xi_rx_configuration - RX configuration                                   */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_BBH_ERROR - Return code                                       */
/*     DRV_BBH_NO_ERROR - No error                                      */
/*     DRV_BBH_INVALID_PORT_INDEX - Invalid port index                  */
/*                                                                            */
/******************************************************************************/
DRV_BBH_ERROR fi_bl_drv_bbh_rx_set_configuration ( DRV_BBH_PORT_INDEX xi_port_index ,
                                                          const DRV_BBH_RX_CONFIGURATION * xi_rx_configuration )
{
    BBH_RX_GENERAL_CONFIGURATION_BBCFG rx_bbcfg ;
    BBH_RX_GENERAL_CONFIGURATION_BBCFG1 rx_bbcfg1 ;
    BBH_RX_GENERAL_CONFIGURATION_DDRCFG rx_ddrcfg ;
    BBH_RX_GENERAL_CONFIGURATION_PDBASE rx_pdbase ;
    BBH_RX_GENERAL_CONFIGURATION_PDSIZE rx_pdsize ;
    BBH_RX_GENERAL_CONFIGURATION_RUNNERTASK rx_runnertask ;
    BBH_RX_GENERAL_CONFIGURATION_DMAADDR rx_dmaaddr ;
    BBH_RX_GENERAL_CONFIGURATION_DMACFG rx_dmacfg ;
    BBH_RX_GENERAL_CONFIGURATION_SDMAADDR rx_sdmaaddr ;
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG rx_sdmacfg ;
    BBH_RX_GENERAL_CONFIGURATION_MINPKT0 rx_minpkt0 ;
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT0 rx_maxpkt0 ;
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT1 rx_maxpkt1 ;
    BBH_RX_GENERAL_CONFIGURATION_IHCFG rx_ihcfg ;
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH rx_perflowth ;
    BBH_RX_GENERAL_CONFIGURATION_PLOAMCFG rx_ploamcfg ;
    BBH_RX_GENERAL_CONFIGURATION_REASSEMBLYOFFSET rx_reassemblyoffset ;
    /* new registers for DOCSIS BBH RX */
    BBH_RX_GENERAL_CONFIGURATION_PPCFG rx_pp_cfg;
    BBH_RX_GENERAL_CONFIGURATION_PPTASK0 rx_pp_task0;
    BBH_RX_GENERAL_CONFIGURATION_PPTASK1 rx_pp_task1;
    BBH_RX_GENERAL_CONFIGURATION_MISC_CFG rx_misc_cfg;
    BBH_RX_GENERAL_CONFIGURATION_DDR2CFG rx_ddr2cfg;


    DRV_BBH_ERROR error_code ;

    if ( xi_port_index >= DRV_BBH_NUMBER_OF_PORTS )
    {
        return ( DRV_BBH_INVALID_PORT_INDEX ) ;
    }

    if ( ( xi_rx_configuration->pd_fifo_size_normal_queue < DRV_BBH_RX_MINIMAL_PD_FIFO_SIZE ) ||
         ( xi_rx_configuration->pd_fifo_size_normal_queue > DRV_BBH_RX_MAXIMAL_PD_FIFO_SIZE ) )
    {
        return ( DRV_BBH_INVALID_RX_PD_FIFO_SIZE ) ;
    }

    if ( ( xi_rx_configuration->pd_fifo_size_direct_queue < DRV_BBH_RX_MINIMAL_PD_FIFO_SIZE ) ||
         ( xi_rx_configuration->pd_fifo_size_direct_queue > DRV_BBH_RX_MAXIMAL_PD_FIFO_SIZE ) )
    {
        return ( DRV_BBH_INVALID_RX_PD_FIFO_SIZE ) ;
    }

    if ( ( xi_rx_configuration->dma_data_and_chunk_descriptor_fifos_size < DRV_BBH_RX_MINIMAL_DMA_OR_SDMA_DATA_FIFO_SIZE ) ||
         ( xi_rx_configuration->dma_data_and_chunk_descriptor_fifos_size > DRV_BBH_RX_MAXIMAL_DMA_OR_SDMA_DATA_FIFO_SIZE ) )
    {
        return ( DRV_BBH_INVALID_DMA_DATA_AND_CHUNK_DESCRIPTOR_FIFOS_SIZE ) ;
    }

    if ( ( xi_rx_configuration->dma_exclusive_threshold < DRV_BBH_RX_MINIMAL_DMA_OR_SDMA_DATA_FIFO_SIZE ) ||
         ( xi_rx_configuration->dma_exclusive_threshold > DRV_BBH_RX_MAXIMAL_DMA_OR_SDMA_DATA_FIFO_SIZE ) )
    {
        return ( DRV_BBH_INVALID_DMA_EXCLUSIVE_THRESHOLD ) ;
    }

    if ( ( xi_rx_configuration->sdma_data_and_chunk_descriptor_fifos_size < DRV_BBH_RX_MINIMAL_DMA_OR_SDMA_DATA_FIFO_SIZE ) ||
         ( xi_rx_configuration->sdma_data_and_chunk_descriptor_fifos_size > DRV_BBH_RX_MAXIMAL_DMA_OR_SDMA_DATA_FIFO_SIZE ) )
    {
        return ( DRV_BBH_INVALID_SDMA_DATA_AND_CHUNK_DESCRIPTOR_FIFOS_SIZE ) ;
    }

    if ( ( xi_rx_configuration->sdma_exclusive_threshold < DRV_BBH_RX_MINIMAL_DMA_OR_SDMA_DATA_FIFO_SIZE ) ||
         ( xi_rx_configuration->sdma_exclusive_threshold > DRV_BBH_RX_MAXIMAL_DMA_OR_SDMA_DATA_FIFO_SIZE ) )
    {
        return ( DRV_BBH_INVALID_SDMA_EXCLUSIVE_THRESHOLD ) ;
    }

    if ( ( ! f_minimum_packet_size_is_valid ( xi_rx_configuration->minimum_packet_size_0 , xi_rx_configuration->packet_header_offset ) ) ||
         ( ! f_minimum_packet_size_is_valid ( xi_rx_configuration->minimum_packet_size_1 , xi_rx_configuration->packet_header_offset ) ) ||
         ( ! f_minimum_packet_size_is_valid ( xi_rx_configuration->minimum_packet_size_2 , xi_rx_configuration->packet_header_offset ) ) ||
         ( ! f_minimum_packet_size_is_valid ( xi_rx_configuration->minimum_packet_size_3 , xi_rx_configuration->packet_header_offset ) ) )
    {
        return ( DRV_BBH_INVALID_MINIMUM_PACKET_SIZE ) ;
    }

    if ( ( ! f_maximum_packet_size_is_valid ( xi_rx_configuration->maximum_packet_size_0 ,
                                              xi_rx_configuration->packet_header_offset ,
                                              xi_rx_configuration->reassembly_offset_in_8_byte ) ) ||
         ( ! f_maximum_packet_size_is_valid ( xi_rx_configuration->maximum_packet_size_1 ,
                                              xi_rx_configuration->packet_header_offset ,
                                              xi_rx_configuration->reassembly_offset_in_8_byte ) ) ||
         ( ! f_maximum_packet_size_is_valid ( xi_rx_configuration->maximum_packet_size_2 ,
                                              xi_rx_configuration->packet_header_offset ,
                                              xi_rx_configuration->reassembly_offset_in_8_byte ) ) ||
         ( ! f_maximum_packet_size_is_valid ( xi_rx_configuration->maximum_packet_size_3 ,
                                              xi_rx_configuration->packet_header_offset ,
                                              xi_rx_configuration->reassembly_offset_in_8_byte ) ) )
    {
        return ( DRV_BBH_INVALID_MAXIMUM_PACKET_SIZE ) ;
    }

    if ( xi_rx_configuration->packet_header_offset > DRV_BBH_RX_MAXIMAL_PACKET_HEADER_OFFSET )
    {
        return ( DRV_BBH_INVALID_PACKET_HEADER_OFFSET ) ;
    }

    if ( xi_rx_configuration->flows_32_255_group_divider < DRV_BBH_RX_MINIMAL_FLOWS_32_255_GROUP_DIVIDER )
    {
        return ( DRV_BBH_INVALID_FLOWS_32_255_GROUP_DIVIDER ) ;
    }

    if ( xi_rx_configuration->ploam_default_ih_class > DRV_BBH_RX_MAXIMAL_IH_CLASS )
    {
        return ( DRV_BBH_INVALID_IH_CLASS ) ;
    }

    if ( xi_rx_configuration->reassembly_offset_in_8_byte > DRV_BBH_RX_MAXIMAL_REASSEMBLY_OFFSET_IN_8_BYTE )
    {
        return ( DRV_BBH_INVALID_REASSEMBLY_OFFSET ) ;
    }


    BBH_RX_GENERAL_CONFIGURATION_BBCFG_READ( xi_port_index , rx_bbcfg ) ;
    rx_bbcfg.runner0route = xi_rx_configuration->runner_0_route_address ;
    rx_bbcfg.runner1route = xi_rx_configuration->runner_1_route_address ;
    rx_bbcfg.dmaroute = xi_rx_configuration->dma_route_address ;
    rx_bbcfg.bpmroute = xi_rx_configuration->bpm_route_address ;
    BBH_RX_GENERAL_CONFIGURATION_BBCFG_WRITE( xi_port_index , rx_bbcfg ) ;

    BBH_RX_GENERAL_CONFIGURATION_BBCFG1_READ( xi_port_index , rx_bbcfg1 ) ;
    rx_bbcfg1.ihroute = xi_rx_configuration->ih_route_address ;
    rx_bbcfg1.sdmaroute = xi_rx_configuration->sdma_route_address ;
    rx_bbcfg1.sbpmroute = xi_rx_configuration->sbpm_route_address ;
    BBH_RX_GENERAL_CONFIGURATION_BBCFG1_WRITE( xi_port_index , rx_bbcfg1 ) ;

    /* first DDR bank configuration */
    BBH_RX_GENERAL_CONFIGURATION_DDRCFG_READ( xi_port_index , rx_ddrcfg ) ;
    rx_ddrcfg.bufsize = xi_rx_configuration->ddr_buffer_size ;
    rx_ddrcfg.ddrtmbase = xi_rx_configuration->ddr1_tm_base_address ;
    BBH_RX_GENERAL_CONFIGURATION_DDRCFG_WRITE( xi_port_index , rx_ddrcfg ) ;

    /* second DDR bank configuration */
    BBH_RX_GENERAL_CONFIGURATION_DDR2CFG_READ( xi_port_index , rx_ddr2cfg ) ;
	rx_ddr2cfg.ddrtmbase = xi_rx_configuration->ddr2_tm_base_address ;
	BBH_RX_GENERAL_CONFIGURATION_DDR2CFG_WRITE( xi_port_index , rx_ddr2cfg ) ;


    BBH_RX_GENERAL_CONFIGURATION_PDBASE_READ( xi_port_index , rx_pdbase ) ;
    rx_pdbase.normal = xi_rx_configuration->pd_fifo_base_address_normal_queue_in_8_byte ;
    rx_pdbase.direct = xi_rx_configuration->pd_fifo_base_address_direct_queue_in_8_byte ;
    BBH_RX_GENERAL_CONFIGURATION_PDBASE_WRITE( xi_port_index , rx_pdbase ) ;

    BBH_RX_GENERAL_CONFIGURATION_PDSIZE_READ( xi_port_index , rx_pdsize ) ;
    rx_pdsize.normal = xi_rx_configuration->pd_fifo_size_normal_queue ;
    rx_pdsize.direct = xi_rx_configuration->pd_fifo_size_direct_queue ;
    BBH_RX_GENERAL_CONFIGURATION_PDSIZE_WRITE( xi_port_index , rx_pdsize ) ;

    BBH_RX_GENERAL_CONFIGURATION_RUNNERTASK_READ( xi_port_index , rx_runnertask ) ;
    rx_runnertask.normal0 = xi_rx_configuration->runner_0_task_normal_queue ;
    rx_runnertask.direct0 = xi_rx_configuration->runner_0_task_direct_queue ;
    rx_runnertask.normal1 = xi_rx_configuration->runner_1_task_normal_queue ;
    rx_runnertask.direct1 = xi_rx_configuration->runner_1_task_direct_queue ;
    BBH_RX_GENERAL_CONFIGURATION_RUNNERTASK_WRITE( xi_port_index , rx_runnertask ) ;

    BBH_RX_GENERAL_CONFIGURATION_DMAADDR_READ( xi_port_index , rx_dmaaddr ) ;
    rx_dmaaddr.database = xi_rx_configuration->dma_data_fifo_base_address ;
    rx_dmaaddr.descbase = xi_rx_configuration->dma_chunk_descriptor_fifo_base_address ;
    BBH_RX_GENERAL_CONFIGURATION_DMAADDR_WRITE( xi_port_index , rx_dmaaddr ) ;

    BBH_RX_GENERAL_CONFIGURATION_DMACFG_READ( xi_port_index , rx_dmacfg ) ;
    rx_dmacfg.numofcd = xi_rx_configuration->dma_data_and_chunk_descriptor_fifos_size ;
    rx_dmacfg.exclth = xi_rx_configuration->dma_exclusive_threshold ;
    BBH_RX_GENERAL_CONFIGURATION_DMACFG_WRITE( xi_port_index , rx_dmacfg ) ;

    BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_READ( xi_port_index , rx_sdmaaddr ) ;
    rx_sdmaaddr.database = xi_rx_configuration->sdma_data_fifo_base_address ;
    rx_sdmaaddr.descbase = xi_rx_configuration->sdma_chunk_descriptor_fifo_base_address ;
    BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_WRITE( xi_port_index , rx_sdmaaddr ) ;

    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_READ( xi_port_index , rx_sdmacfg ) ;
    rx_sdmacfg.numofcd = xi_rx_configuration->sdma_data_and_chunk_descriptor_fifos_size ;
    rx_sdmacfg.exclth = xi_rx_configuration->sdma_exclusive_threshold ;
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_WRITE( xi_port_index , rx_sdmacfg ) ;

    BBH_RX_GENERAL_CONFIGURATION_MINPKT0_READ( xi_port_index , rx_minpkt0 ) ;
    rx_minpkt0.minpkt0 = xi_rx_configuration->minimum_packet_size_0 ;
    rx_minpkt0.minpkt1 = xi_rx_configuration->minimum_packet_size_1 ;
    rx_minpkt0.minpkt2 = xi_rx_configuration->minimum_packet_size_2 ;
    rx_minpkt0.minpkt3 = xi_rx_configuration->minimum_packet_size_3 ;
    BBH_RX_GENERAL_CONFIGURATION_MINPKT0_WRITE( xi_port_index , rx_minpkt0 ) ;

    BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_READ( xi_port_index , rx_maxpkt0 ) ;
    rx_maxpkt0.maxpkt0 = xi_rx_configuration->maximum_packet_size_0 ;
    rx_maxpkt0.maxpkt1 = xi_rx_configuration->maximum_packet_size_1 ;
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_WRITE( xi_port_index , rx_maxpkt0 ) ;

    BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_READ( xi_port_index , rx_maxpkt1 ) ;
    rx_maxpkt1.maxpkt2 = xi_rx_configuration->maximum_packet_size_2 ;
    rx_maxpkt1.maxpkt3 = xi_rx_configuration->maximum_packet_size_3 ;
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_WRITE( xi_port_index , rx_maxpkt1 ) ;

    BBH_RX_GENERAL_CONFIGURATION_IHCFG_READ( xi_port_index , rx_ihcfg ) ;
    rx_ihcfg.ihbufen = xi_rx_configuration->ih_ingress_buffers_bitmask ;
    rx_ihcfg.sopoffset = xi_rx_configuration->packet_header_offset ;
    BBH_RX_GENERAL_CONFIGURATION_IHCFG_WRITE( xi_port_index , rx_ihcfg ) ;


    error_code = fi_bl_drv_bbh_rx_set_triggers_of_flow_control_and_drop ( xi_port_index ,
                                                                                xi_rx_configuration->flow_control_triggers_bitmask ,
                                                                                xi_rx_configuration->drop_triggers_bitmask ) ;
    if ( error_code != DRV_BBH_NO_ERROR )
    {
        return ( error_code ) ;
    }


    BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_READ( xi_port_index , rx_perflowth ) ;
    rx_perflowth.flowth = xi_rx_configuration->flows_32_255_group_divider ;
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_WRITE( xi_port_index , rx_perflowth ) ;

    BBH_RX_GENERAL_CONFIGURATION_PLOAMCFG_READ( xi_port_index , rx_ploamcfg ) ;
    rx_ploamcfg.ihclass = xi_rx_configuration->ploam_default_ih_class ;
    rx_ploamcfg.ihoverride = xi_rx_configuration->ploam_ih_class_override ;
    BBH_RX_GENERAL_CONFIGURATION_PLOAMCFG_WRITE( xi_port_index , rx_ploamcfg ) ;

    BBH_RX_GENERAL_CONFIGURATION_REASSEMBLYOFFSET_READ( xi_port_index , rx_reassemblyoffset ) ;
    rx_reassemblyoffset.offset = xi_rx_configuration->reassembly_offset_in_8_byte ;
    BBH_RX_GENERAL_CONFIGURATION_REASSEMBLYOFFSET_WRITE( xi_port_index , rx_reassemblyoffset ) ;

    BBH_RX_GENERAL_CONFIGURATION_MISC_CFG_READ( xi_port_index , rx_misc_cfg ) ;
    rx_misc_cfg.bb_split_cfg = BBH_RX_DOCSIS_BYOI_BB_SPLIT;
    rx_misc_cfg.buf_size_sel = xi_rx_configuration->fpm_buff_size_set;
	BBH_RX_GENERAL_CONFIGURATION_MISC_CFG_WRITE( xi_port_index , rx_misc_cfg ) ;

    /* in case of WAN configure the parallel processing */
    if ( xi_port_index == DRV_BBH_WAN)
    {

	BBH_RX_GENERAL_CONFIGURATION_PPCFG_READ(xi_port_index, rx_pp_cfg);
	/* For now VLSI suggest to enable only runner 0 normal task */
#if 0 /* hardware dispatch is not currently being used */
	rx_pp_cfg.rnr0normal = 1;
#endif
	rx_pp_cfg.tasken = xi_rx_configuration->pp_task_enable_bitmap;
	BBH_RX_GENERAL_CONFIGURATION_PPCFG_WRITE(xi_port_index, rx_pp_cfg);

	BBH_RX_GENERAL_CONFIGURATION_PPTASK0_READ(xi_port_index, rx_pp_task0);
	BBH_RX_GENERAL_CONFIGURATION_PPTASK1_READ(xi_port_index, rx_pp_task1);
	rx_pp_task0.pptask1 = xi_rx_configuration->pp_task_nums[0];
	rx_pp_task0.pptask2 = xi_rx_configuration->pp_task_nums[1];
	rx_pp_task0.pptask3 = xi_rx_configuration->pp_task_nums[2];
	rx_pp_task1.pptask4 = xi_rx_configuration->pp_task_nums[3];
	rx_pp_task1.pptask5 = xi_rx_configuration->pp_task_nums[4];
	rx_pp_task1.pptask6 = xi_rx_configuration->pp_task_nums[5];
	rx_pp_task1.pptask7 = xi_rx_configuration->pp_task_nums[6];
	BBH_RX_GENERAL_CONFIGURATION_PPTASK0_WRITE(xi_port_index, rx_pp_task0);
		BBH_RX_GENERAL_CONFIGURATION_PPTASK1_WRITE(xi_port_index, rx_pp_task1);
    }
    return ( DRV_BBH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_bbh_rx_set_configuration ) ;

#ifndef _CFE_
/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bbh_rx_get_configuration                                       */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BBH Driver - RX Get configuration                                        */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the configuration of the RX part of BBH block.        */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_port_index - Port index                                               */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_rx_configuration - RX configuration                                   */
/*                                                                            */
/*   DRV_BBH_ERROR - Return code                                       */
/*     DRV_BBH_NO_ERROR - No error                                      */
/*     DRV_BBH_INVALID_PORT_INDEX - Invalid port index                  */
/*                                                                            */
/******************************************************************************/
DRV_BBH_ERROR fi_bl_drv_bbh_rx_get_configuration ( DRV_BBH_PORT_INDEX xi_port_index ,
                                                          DRV_BBH_RX_CONFIGURATION * const xo_rx_configuration )
{
    BBH_RX_GENERAL_CONFIGURATION_BBCFG rx_bbcfg ;
    BBH_RX_GENERAL_CONFIGURATION_BBCFG1 rx_bbcfg1 ;
    BBH_RX_GENERAL_CONFIGURATION_DDRCFG rx_ddrcfg ;
    BBH_RX_GENERAL_CONFIGURATION_PDBASE rx_pdbase ;
    BBH_RX_GENERAL_CONFIGURATION_PDSIZE rx_pdsize ;
    BBH_RX_GENERAL_CONFIGURATION_RUNNERTASK rx_runnertask ;
    BBH_RX_GENERAL_CONFIGURATION_DMAADDR rx_dmaaddr ;
    BBH_RX_GENERAL_CONFIGURATION_DMACFG rx_dmacfg ;
    BBH_RX_GENERAL_CONFIGURATION_SDMAADDR rx_sdmaaddr ;
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG rx_sdmacfg ;
    BBH_RX_GENERAL_CONFIGURATION_MINPKT0 rx_minpkt0 ;
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT0 rx_maxpkt0 ;
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT1 rx_maxpkt1 ;
    BBH_RX_GENERAL_CONFIGURATION_IHCFG rx_ihcfg ;
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH rx_perflowth ;
    BBH_RX_GENERAL_CONFIGURATION_PLOAMCFG rx_ploamcfg ;
    BBH_RX_GENERAL_CONFIGURATION_REASSEMBLYOFFSET rx_reassemblyoffset ;
    /* new registers for DOCSIS BBH RX */
    BBH_RX_GENERAL_CONFIGURATION_PPCFG rx_pp_cfg;
    BBH_RX_GENERAL_CONFIGURATION_PPTASK0 rx_pp_task0;
    BBH_RX_GENERAL_CONFIGURATION_PPTASK1 rx_pp_task1;
    BBH_RX_GENERAL_CONFIGURATION_MISC_CFG rx_misc_cfg;
    BBH_RX_GENERAL_CONFIGURATION_DDR2CFG rx_ddr2cfg;
    DRV_BBH_ERROR error_code ;

    if ( xi_port_index >= DRV_BBH_NUMBER_OF_PORTS )
    {
        return ( DRV_BBH_INVALID_PORT_INDEX ) ;
    }


    BBH_RX_GENERAL_CONFIGURATION_BBCFG_READ( xi_port_index , rx_bbcfg ) ;
    xo_rx_configuration->runner_0_route_address = rx_bbcfg.runner0route ;
    xo_rx_configuration->runner_1_route_address = rx_bbcfg.runner1route ;
    xo_rx_configuration->dma_route_address = rx_bbcfg.dmaroute ;
    xo_rx_configuration->bpm_route_address = rx_bbcfg.bpmroute ;

    BBH_RX_GENERAL_CONFIGURATION_BBCFG1_READ( xi_port_index , rx_bbcfg1 ) ;
    xo_rx_configuration->ih_route_address = rx_bbcfg1.ihroute ;
    xo_rx_configuration->sdma_route_address = rx_bbcfg1.sdmaroute ;
    xo_rx_configuration->sbpm_route_address = rx_bbcfg1.sbpmroute ;

    BBH_RX_GENERAL_CONFIGURATION_DDRCFG_READ( xi_port_index , rx_ddrcfg ) ;
    xo_rx_configuration->ddr_buffer_size = rx_ddrcfg.bufsize ;
    xo_rx_configuration->ddr1_tm_base_address = rx_ddrcfg.ddrtmbase ;

    BBH_RX_GENERAL_CONFIGURATION_DDR2CFG_READ( xi_port_index , rx_ddr2cfg ) ;
	xo_rx_configuration->ddr2_tm_base_address = rx_ddr2cfg.ddrtmbase ;

    BBH_RX_GENERAL_CONFIGURATION_PDBASE_READ( xi_port_index , rx_pdbase ) ;
    xo_rx_configuration->pd_fifo_base_address_normal_queue_in_8_byte = rx_pdbase.normal ;
    xo_rx_configuration->pd_fifo_base_address_direct_queue_in_8_byte = rx_pdbase.direct ;

    BBH_RX_GENERAL_CONFIGURATION_PDSIZE_READ( xi_port_index , rx_pdsize ) ;
    xo_rx_configuration->pd_fifo_size_normal_queue = rx_pdsize.normal ;
    xo_rx_configuration->pd_fifo_size_direct_queue = rx_pdsize.direct ;

    BBH_RX_GENERAL_CONFIGURATION_RUNNERTASK_READ( xi_port_index , rx_runnertask ) ;
    xo_rx_configuration->runner_0_task_normal_queue = rx_runnertask.normal0 ;
    xo_rx_configuration->runner_0_task_direct_queue = rx_runnertask.direct0 ;
    xo_rx_configuration->runner_1_task_normal_queue = rx_runnertask.normal1 ;
    xo_rx_configuration->runner_1_task_direct_queue = rx_runnertask.direct1 ;

    BBH_RX_GENERAL_CONFIGURATION_DMAADDR_READ( xi_port_index , rx_dmaaddr ) ;
    xo_rx_configuration->dma_data_fifo_base_address = rx_dmaaddr.database ;
    xo_rx_configuration->dma_chunk_descriptor_fifo_base_address = rx_dmaaddr.descbase ;

    BBH_RX_GENERAL_CONFIGURATION_DMACFG_READ( xi_port_index , rx_dmacfg ) ;
    xo_rx_configuration->dma_data_and_chunk_descriptor_fifos_size = rx_dmacfg.numofcd ;
    xo_rx_configuration->dma_exclusive_threshold = rx_dmacfg.exclth ;

    BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_READ( xi_port_index , rx_sdmaaddr ) ;
    xo_rx_configuration->sdma_data_fifo_base_address = rx_sdmaaddr.database ;
    xo_rx_configuration->sdma_chunk_descriptor_fifo_base_address = rx_sdmaaddr.descbase ;

    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_READ( xi_port_index , rx_sdmacfg ) ;
    xo_rx_configuration->sdma_data_and_chunk_descriptor_fifos_size = rx_sdmacfg.numofcd ;
    xo_rx_configuration->sdma_exclusive_threshold = rx_sdmacfg.exclth ;

    BBH_RX_GENERAL_CONFIGURATION_MINPKT0_READ( xi_port_index , rx_minpkt0 ) ;
    xo_rx_configuration->minimum_packet_size_0 = rx_minpkt0.minpkt0 ;
    xo_rx_configuration->minimum_packet_size_1 = rx_minpkt0.minpkt1 ;
    xo_rx_configuration->minimum_packet_size_2 = rx_minpkt0.minpkt2 ;
    xo_rx_configuration->minimum_packet_size_3 = rx_minpkt0.minpkt3 ;

    BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_READ( xi_port_index , rx_maxpkt0 ) ;
    xo_rx_configuration->maximum_packet_size_0 = rx_maxpkt0.maxpkt0 ;
    xo_rx_configuration->maximum_packet_size_1 = rx_maxpkt0.maxpkt1 ;

    BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_READ( xi_port_index , rx_maxpkt1 ) ;
    xo_rx_configuration->maximum_packet_size_2 = rx_maxpkt1.maxpkt2 ;
    xo_rx_configuration->maximum_packet_size_3 = rx_maxpkt1.maxpkt3 ;

    BBH_RX_GENERAL_CONFIGURATION_IHCFG_READ( xi_port_index , rx_ihcfg ) ;
    xo_rx_configuration->ih_ingress_buffers_bitmask = rx_ihcfg.ihbufen ;
    xo_rx_configuration->packet_header_offset = rx_ihcfg.sopoffset ;



    error_code = fi_bl_drv_bbh_rx_get_triggers_of_flow_control_and_drop ( xi_port_index ,
                                                                                & xo_rx_configuration->flow_control_triggers_bitmask ,
                                                                                & xo_rx_configuration->drop_triggers_bitmask ) ;
    if ( error_code != DRV_BBH_NO_ERROR )
    {
        return ( error_code ) ;
    }


    BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_READ( xi_port_index , rx_perflowth ) ;
    xo_rx_configuration->flows_32_255_group_divider = rx_perflowth.flowth ;

    BBH_RX_GENERAL_CONFIGURATION_PLOAMCFG_READ( xi_port_index , rx_ploamcfg ) ;
    xo_rx_configuration->ploam_default_ih_class = rx_ploamcfg.ihclass ;
    xo_rx_configuration->ploam_ih_class_override = rx_ploamcfg.ihoverride ;

    BBH_RX_GENERAL_CONFIGURATION_REASSEMBLYOFFSET_READ( xi_port_index , rx_reassemblyoffset ) ;
    xo_rx_configuration->reassembly_offset_in_8_byte = rx_reassemblyoffset.offset ;

    BBH_RX_GENERAL_CONFIGURATION_MISC_CFG_READ(xi_port_index, rx_misc_cfg);
    xo_rx_configuration->fpm_buff_size_set = (DRV_BBH_FPM_BUFF_SIZE)rx_misc_cfg.buf_size_sel;

    BBH_RX_GENERAL_CONFIGURATION_PPCFG_READ(xi_port_index, rx_pp_cfg);
    xo_rx_configuration->pp_task_enable_bitmap = rx_pp_cfg.tasken;

    BBH_RX_GENERAL_CONFIGURATION_PPTASK0_READ(xi_port_index, rx_pp_task0);
    BBH_RX_GENERAL_CONFIGURATION_PPTASK1_READ(xi_port_index, rx_pp_task1);
    xo_rx_configuration->pp_task_nums[0] = rx_pp_task0.pptask1;
    xo_rx_configuration->pp_task_nums[1] = rx_pp_task0.pptask2;
    xo_rx_configuration->pp_task_nums[2] = rx_pp_task0.pptask3;
    xo_rx_configuration->pp_task_nums[3] = rx_pp_task1.pptask4;
    xo_rx_configuration->pp_task_nums[4] = rx_pp_task1.pptask5;
    xo_rx_configuration->pp_task_nums[5] = rx_pp_task1.pptask6;
    xo_rx_configuration->pp_task_nums[6] = rx_pp_task1.pptask7;

    return ( DRV_BBH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_bbh_rx_get_configuration ) ;

#endif
/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bbh_rx_set_triggers_of_flow_control_and_drop                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BBH Driver - RX Set triggers for flow control and drop                   */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets triggers for sending flow control to MAC, and         */
/*   triggers for dropping packets. For flow control, there are 3 possible    */
/*   triggers: BPM is in exclusive state, SBPM is in exclusive state, Runner  */
/*   request. For drop, there are 2 possible triggers: BPM is in exclusive    */
/*   state, SBPM is in exclusive state. The triggers are turned on/off        */
/*   according to the given bitmask. Values of the enumeration                */
/*   DRV_BBH_RX_FLOW_CONTROL_TRIGGER should be ORed, as a        */
/*   description of the desired triggers for flow control. Values of the      */
/*   enumeration DRV_BBH_RX_DROP_TRIGGER should be ORed, as a    */
/*   description of the desired triggers for drop.                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_port_index - Port index                                               */
/*                                                                            */
/*   xi_flow_control_triggers_bitmask - Flow control triggers bitmask         */
/*                                                                            */
/*   xi_drop_triggers_bitmask - Drop triggers bitmask                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_BBH_ERROR - Return code                                       */
/*     DRV_BBH_NO_ERROR - No error                                      */
/*     DRV_BBH_INVALID_PORT_INDEX - Invalid port index                  */
/*                                                                            */
/******************************************************************************/
DRV_BBH_ERROR fi_bl_drv_bbh_rx_set_triggers_of_flow_control_and_drop ( DRV_BBH_PORT_INDEX xi_port_index ,
                                                                              uint8_t xi_flow_control_triggers_bitmask ,
                                                                              uint8_t xi_drop_triggers_bitmask )
{
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL rx_flowctrl ;
    int32_t flow_control_trigger_bpm_is_in_exclusive_state_enable ;
    int32_t flow_control_trigger_sbpm_is_in_exclusive_state_enable ;
    int32_t flow_control_trigger_runner_request_enable ;
    int32_t drop_trigger_bpm_is_in_exclusive_state_enable ;
    int32_t drop_trigger_sbpm_is_in_exclusive_state_enable ;

    if ( xi_port_index >= DRV_BBH_NUMBER_OF_PORTS )
    {
        return ( DRV_BBH_INVALID_PORT_INDEX ) ;
    }


    if ( ( xi_flow_control_triggers_bitmask & DRV_BBH_RX_FLOW_CONTROL_TRIGGER_BPM_IS_IN_EXCLUSIVE_STATE ) == 0 )
    {
        flow_control_trigger_bpm_is_in_exclusive_state_enable =  0 ;
    }
    else
    {
        flow_control_trigger_bpm_is_in_exclusive_state_enable =  1 ;
    }

    if ( ( xi_flow_control_triggers_bitmask & DRV_BBH_RX_FLOW_CONTROL_TRIGGER_SBPM_IS_IN_EXCLUSIVE_STATE ) == 0 )
    {
        flow_control_trigger_sbpm_is_in_exclusive_state_enable =  0 ;
    }
    else
    {
        flow_control_trigger_sbpm_is_in_exclusive_state_enable =  1 ;
    }

    if ( ( xi_flow_control_triggers_bitmask & DRV_BBH_RX_FLOW_CONTROL_TRIGGER_RUNNER_REQUEST ) == 0 )
    {
        flow_control_trigger_runner_request_enable =  0 ;
    }
    else
    {
        flow_control_trigger_runner_request_enable =  1 ;
    }


    if ( ( xi_drop_triggers_bitmask & DRV_BBH_RX_DROP_TRIGGER_BPM_IS_IN_EXCLUSIVE_STATE ) == 0 )
    {
        drop_trigger_bpm_is_in_exclusive_state_enable =  0 ;
    }
    else
    {
        drop_trigger_bpm_is_in_exclusive_state_enable =  1 ;
    }

    if ( ( xi_drop_triggers_bitmask & DRV_BBH_RX_DROP_TRIGGER_SBPM_IS_IN_EXCLUSIVE_STATE ) == 0 )
    {
        drop_trigger_sbpm_is_in_exclusive_state_enable =  0 ;
    }
    else
    {
        drop_trigger_sbpm_is_in_exclusive_state_enable =  1 ;
    }


    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_READ( xi_port_index , rx_flowctrl ) ;
    rx_flowctrl.bpmen = flow_control_trigger_bpm_is_in_exclusive_state_enable ;
    rx_flowctrl.sbpmen = flow_control_trigger_sbpm_is_in_exclusive_state_enable ;
    rx_flowctrl.runneren = flow_control_trigger_runner_request_enable ;
    rx_flowctrl.bpmdropen = drop_trigger_bpm_is_in_exclusive_state_enable ;
    rx_flowctrl.sbpmdropen = drop_trigger_sbpm_is_in_exclusive_state_enable ;
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_WRITE( xi_port_index , rx_flowctrl ) ;

    return ( DRV_BBH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_bbh_rx_set_triggers_of_flow_control_and_drop ) ;

#ifndef _CFE_
/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bbh_rx_get_triggers_of_flow_control_and_drop                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BBH Driver - RX Get triggers for flow control and drop                   */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the triggers for sending flow control to MAC, and the */
/*   triggers for dropping packets. For flow control, there are 3 possible    */
/*   triggers: BPM is in exclusive state, SBPM is in exclusive state, Runner  */
/*   request. For drop, there are 2 possible triggers: BPM is in exclusive    */
/*   state, SBPM is in exclusive state. The triggers statuses are given in a  */
/*   bitmask, according to DRV_BBH_RX_FLOW_CONTROL_TRIGGER and         */
/*   DRV_BBH_RX_DROP_TRIGGER enumerations.                             */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_port_index - Port index                                               */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_flow_control_triggers_bitmask - Flow control triggers bitmask         */
/*                                                                            */
/*   xo_drop_triggers_bitmask - Drop triggers bitmask                         */
/*                                                                            */
/*   DRV_BBH_ERROR - Return code                                       */
/*     DRV_BBH_NO_ERROR - No error                                      */
/*     DRV_BBH_INVALID_PORT_INDEX - Invalid port index                  */
/*                                                                            */
/******************************************************************************/
DRV_BBH_ERROR fi_bl_drv_bbh_rx_get_triggers_of_flow_control_and_drop ( DRV_BBH_PORT_INDEX xi_port_index ,
                                                                              uint8_t * const xo_flow_control_triggers_bitmask ,
                                                                              uint8_t * const xo_drop_triggers_bitmask )
{
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL rx_flowctrl ;

    if ( xi_port_index >= DRV_BBH_NUMBER_OF_PORTS )
    {
        return ( DRV_BBH_INVALID_PORT_INDEX ) ;
    }


    * xo_flow_control_triggers_bitmask = 0 ;
    * xo_drop_triggers_bitmask = 0 ;


    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_READ( xi_port_index , rx_flowctrl ) ;

    if ( rx_flowctrl.bpmen ==  1 )
    {
        * xo_flow_control_triggers_bitmask |= DRV_BBH_RX_FLOW_CONTROL_TRIGGER_BPM_IS_IN_EXCLUSIVE_STATE ;
    }

    if ( rx_flowctrl.sbpmen ==  1 )
    {
        * xo_flow_control_triggers_bitmask |= DRV_BBH_RX_FLOW_CONTROL_TRIGGER_SBPM_IS_IN_EXCLUSIVE_STATE ;
    }

    if ( rx_flowctrl.runneren ==  1 )
    {
        * xo_flow_control_triggers_bitmask |= DRV_BBH_RX_FLOW_CONTROL_TRIGGER_RUNNER_REQUEST ;
    }


    if ( rx_flowctrl.bpmdropen ==  1 )
    {
        * xo_drop_triggers_bitmask |= DRV_BBH_RX_DROP_TRIGGER_BPM_IS_IN_EXCLUSIVE_STATE ;
    }

    if ( rx_flowctrl.sbpmdropen ==  1 )
    {
        * xo_drop_triggers_bitmask |= DRV_BBH_RX_DROP_TRIGGER_SBPM_IS_IN_EXCLUSIVE_STATE ;
    }


    return ( DRV_BBH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_bbh_rx_get_triggers_of_flow_control_and_drop ) ;

#endif
/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bbh_rx_set_per_flow_configuration                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BBH Driver - RX Set per flow configuration                               */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets per-flow configuration in the RX part of BBH block.   */
/*   Each one of flows 0-31 has its own configuration. Flows 32-255 are       */
/*   divided into 2 groups (32 to x, x+1 to 255). Each group has its own      */
/*   configuration. The groups-divider (x) is configured in "RX Set           */
/*   configuration" API. In Ethernet case, only flow 0 is relevant.           */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_port_index - Port index                                               */
/*                                                                            */
/*   xi_flow_index - Flow index                                               */
/*                                                                            */
/*   xi_per_flow_configuration - Per flow configuration                       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_BBH_ERROR - Return code                                       */
/*     DRV_BBH_NO_ERROR - No error                                      */
/*     DRV_BBH_INVALID_PORT_INDEX - Invalid port index                  */
/*                                                                            */
/******************************************************************************/
DRV_BBH_ERROR fi_bl_drv_bbh_rx_set_per_flow_configuration ( DRV_BBH_PORT_INDEX xi_port_index ,
                                                                   DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION xi_flow_index ,
                                                                   const DRV_BBH_PER_FLOW_CONFIGURATION * xi_per_flow_configuration )
{
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS rx_perflowsets ;
    BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0 rx_minpktsel ;
    BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0 rx_maxpktsel ;
    BBH_RX_GENERAL_CONFIGURATION_IHCLASS0 rx_ihclass ;
    BBH_RX_GENERAL_CONFIGURATION_IHOVERRIDE rx_ihoverride ;


    if ( xi_port_index >= DRV_BBH_NUMBER_OF_PORTS )
    {
        return ( DRV_BBH_INVALID_PORT_INDEX ) ;
    }

    if ( xi_flow_index > DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_1 )
    {
        return ( DRV_BBH_INVALID_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION ) ;
    }

    /* for Ethernet port, only flow 0 is relevant */
//  if (xi_port_index != DRV_BBH_DOCSIS  && xi_port_index != DRV_BBH_BYOI)
//  {
//      return ( DRV_BBH_ILLEGAL_FLOW_INDEX_FOR_ETHERNET_PORT ) ;
//  }

    if ( xi_per_flow_configuration->minimum_packet_size_selection > DRV_BBH_RX_MAXIMAL_SELECTION_FOR_MINIMUM_OR_MAXIMUM_PACKET_SIZE )
    {
        return ( DRV_BBH_INVALID_MINIMUM_PACKET_SIZE_SELECTION ) ;
    }

    if ( xi_per_flow_configuration->maximum_packet_size_selection > DRV_BBH_RX_MAXIMAL_SELECTION_FOR_MINIMUM_OR_MAXIMUM_PACKET_SIZE )
    {
        return ( DRV_BBH_INVALID_MAXIMUM_PACKET_SIZE_SELECTION ) ;
    }

    if ( xi_per_flow_configuration->default_ih_class > DRV_BBH_RX_MAXIMAL_IH_CLASS )
    {
        return ( DRV_BBH_INVALID_IH_CLASS ) ;
    }


    /* if it is one of the two groups */
    if ( xi_flow_index >= DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_0 )
    {
        BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_READ( xi_port_index , rx_perflowsets ) ;

        switch ( xi_flow_index )
        {
        case DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_0:
            rx_perflowsets.minpktsel0 = xi_per_flow_configuration->minimum_packet_size_selection ;
            rx_perflowsets.maxpktsel0 = xi_per_flow_configuration->maximum_packet_size_selection ;
            rx_perflowsets.ihclass0 = xi_per_flow_configuration->default_ih_class ;
            rx_perflowsets.override0 = xi_per_flow_configuration->ih_class_override ;
            break ;

        case DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_1:
            rx_perflowsets.minpktsel1 = xi_per_flow_configuration->minimum_packet_size_selection ;
            rx_perflowsets.maxpktsel1 = xi_per_flow_configuration->maximum_packet_size_selection ;
            rx_perflowsets.ihclass1 = xi_per_flow_configuration->default_ih_class ;
            rx_perflowsets.override1 = xi_per_flow_configuration->ih_class_override ;
            break ;
        }

        BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_WRITE( xi_port_index , rx_perflowsets ) ;

        DEBUG_TRAafter_init_ih ;

        return ( DRV_BBH_NO_ERROR ) ;
    }


    /* if it is one of flows 0-31: */

    /* minimum & maximum packet length selection */

    /* flows 0-15 */
    if ( xi_flow_index <= CS_RX_MAXIMAL_FLOW_INDEX_FOR_MINPKTSEL0_AND_MAXPKTSEL0_REGISTERS )
    {
        BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_READ( xi_port_index , rx_minpktsel ) ;
        BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_READ( xi_port_index , rx_maxpktsel ) ;
    }
    /* flows 16-31 */
    else
    {
        BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_READ( xi_port_index , rx_minpktsel ) ;
        BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_READ( xi_port_index , rx_maxpktsel ) ;
    }

    MS_SET_BITS( rx_minpktsel.minpktsel ,
                 ( xi_flow_index * CS_RX_FIELD_LENGTH_FOR_MINPKTSEL_AND_MAXPKTSEL_REGISTERS ) % CS_NUMBER_OF_BITS_IN_REGISTER ,
                 CS_RX_FIELD_LENGTH_FOR_MINPKTSEL_AND_MAXPKTSEL_REGISTERS ,
                 xi_per_flow_configuration->minimum_packet_size_selection ) ;

    MS_SET_BITS( rx_maxpktsel.maxpktsel ,
                 ( xi_flow_index * CS_RX_FIELD_LENGTH_FOR_MINPKTSEL_AND_MAXPKTSEL_REGISTERS ) % CS_NUMBER_OF_BITS_IN_REGISTER ,
                 CS_RX_FIELD_LENGTH_FOR_MINPKTSEL_AND_MAXPKTSEL_REGISTERS ,
                 xi_per_flow_configuration->maximum_packet_size_selection ) ;

    /* flows 0-15 */
    if ( xi_flow_index <= CS_RX_MAXIMAL_FLOW_INDEX_FOR_MINPKTSEL0_AND_MAXPKTSEL0_REGISTERS )
    {

        DEBUG_TRAafter_init_ih ;

        BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_WRITE( xi_port_index , rx_minpktsel ) ;

        DEBUG_TRAafter_init_ih ;

        BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_WRITE( xi_port_index , rx_maxpktsel ) ;
    }
    /* flows 16-31 */
    else
    {

        DEBUG_TRAafter_init_ih ;

        BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_WRITE( xi_port_index , rx_minpktsel ) ;

        DEBUG_TRAafter_init_ih ;

        BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_WRITE( xi_port_index , rx_maxpktsel ) ;
    }


    /* default IH class */

    /* flows 0-7 */
    if ( xi_flow_index <= CS_RX_MAXIMAL_FLOW_INDEX_FOR_IHCLASS0_REGISTER )
    {
        BBH_RX_GENERAL_CONFIGURATION_IHCLASS0_READ( xi_port_index , rx_ihclass ) ;
    }
    /* flows 8-15 */
    else if ( xi_flow_index <= CS_RX_MAXIMAL_FLOW_INDEX_FOR_IHCLASS1_REGISTER )
    {
        BBH_RX_GENERAL_CONFIGURATION_IHCLASS1_READ( xi_port_index , rx_ihclass ) ;
    }
    /* flows 16-23 */
    else if ( xi_flow_index <= CS_RX_MAXIMAL_FLOW_INDEX_FOR_IHCLASS2_REGISTER )
    {
        BBH_RX_GENERAL_CONFIGURATION_IHCLASS2_READ( xi_port_index , rx_ihclass ) ;
    }
    /* flows 24-31 */
    else
    {
        BBH_RX_GENERAL_CONFIGURATION_IHCLASS3_READ( xi_port_index , rx_ihclass ) ;
    }


    MS_SET_BITS( rx_ihclass.ihclass ,
                 ( xi_flow_index * CS_RX_FIELD_LENGTH_FOR_IHCLASS_REGISTERS ) % CS_NUMBER_OF_BITS_IN_REGISTER ,
                 CS_RX_FIELD_LENGTH_FOR_IHCLASS_REGISTERS ,
                 xi_per_flow_configuration->default_ih_class ) ;

    /* flows 0-7 */
    if ( xi_flow_index <= CS_RX_MAXIMAL_FLOW_INDEX_FOR_IHCLASS0_REGISTER )
    {
        BBH_RX_GENERAL_CONFIGURATION_IHCLASS0_WRITE( xi_port_index , rx_ihclass ) ;

        DEBUG_TRAafter_init_ih ;

    }
    /* flows 8-15 */
    else if ( xi_flow_index <= CS_RX_MAXIMAL_FLOW_INDEX_FOR_IHCLASS1_REGISTER )
    {
        BBH_RX_GENERAL_CONFIGURATION_IHCLASS1_WRITE( xi_port_index , rx_ihclass ) ;

        DEBUG_TRAafter_init_ih ;

    }
    /* flows 16-23 */
    else if ( xi_flow_index <= CS_RX_MAXIMAL_FLOW_INDEX_FOR_IHCLASS2_REGISTER )
    {
        BBH_RX_GENERAL_CONFIGURATION_IHCLASS2_WRITE( xi_port_index , rx_ihclass ) ;

        DEBUG_TRAafter_init_ih ;

    }
    /* flows 24-31 */
    else
    {
        BBH_RX_GENERAL_CONFIGURATION_IHCLASS3_WRITE( xi_port_index , rx_ihclass ) ;

        DEBUG_TRAafter_init_ih ;

    }


    /* IH class override */

    BBH_RX_GENERAL_CONFIGURATION_IHOVERRIDE_READ( xi_port_index , rx_ihoverride ) ;

    MS_SET_BITS( rx_ihoverride.ihoverride ,
                 xi_flow_index ,
                 1 ,
                 xi_per_flow_configuration->ih_class_override ) ;

    BBH_RX_GENERAL_CONFIGURATION_IHOVERRIDE_WRITE( xi_port_index , rx_ihoverride ) ;

    DEBUG_TRAafter_init_ih ;


    return ( DRV_BBH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_bbh_rx_set_per_flow_configuration ) ;

#ifndef _CFE_
/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bbh_rx_get_per_flow_configuration                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BBH Driver - RX Get per flow configuration                               */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the per-flow configuration in the RX part of BBH      */
/*   block. Each one of flows 0-31 has its own configuration. Flows 32-255    */
/*   are divided into 2 groups (32 to x, x+1 to 255). Each group has its own  */
/*   configuration. The groups-divider (x) is configured in "RX Set per flow  */
/*   configuration" API. In Ethernet case, only flow 0 is relevant.           */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_port_index - Port index                                               */
/*                                                                            */
/*   xi_flow_index - Flow index                                               */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_per_flow_configuration - Per flow configuration                       */
/*                                                                            */
/*   DRV_BBH_ERROR - Return code                                       */
/*     DRV_BBH_NO_ERROR - No error                                      */
/*     DRV_BBH_INVALID_PORT_INDEX - Invalid port index                  */
/*                                                                            */
/******************************************************************************/
DRV_BBH_ERROR fi_bl_drv_bbh_rx_get_per_flow_configuration ( DRV_BBH_PORT_INDEX xi_port_index ,
                                                                   DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION xi_flow_index ,
                                                                   DRV_BBH_PER_FLOW_CONFIGURATION * const xo_per_flow_configuration )
{
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS rx_perflowsets ;
    BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0 rx_minpktsel ;
    BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0 rx_maxpktsel ;
    BBH_RX_GENERAL_CONFIGURATION_IHCLASS0 rx_ihclass ;
    BBH_RX_GENERAL_CONFIGURATION_IHOVERRIDE rx_ihoverride ;

    if ( xi_port_index >= DRV_BBH_NUMBER_OF_PORTS )
    {
        return ( DRV_BBH_INVALID_PORT_INDEX ) ;
    }

    if ( xi_flow_index > DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_1 )
    {
        return ( DRV_BBH_INVALID_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION ) ;
    }

    /* for Ethernet port, only flow 0 is relevant */
    if (xi_port_index != DRV_BBH_WAN)
    {
        return ( DRV_BBH_ILLEGAL_FLOW_INDEX_FOR_ETHERNET_PORT ) ;
    }


    /* if it is one of the two groups */
    if ( xi_flow_index >= DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_0 )
    {
        BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_READ( xi_port_index , rx_perflowsets ) ;

        switch ( xi_flow_index )
        {
        case DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_0:
            xo_per_flow_configuration->minimum_packet_size_selection = rx_perflowsets.minpktsel0 ;
            xo_per_flow_configuration->maximum_packet_size_selection = rx_perflowsets.maxpktsel0 ;
            xo_per_flow_configuration->default_ih_class = rx_perflowsets.ihclass0 ;
            xo_per_flow_configuration->ih_class_override = rx_perflowsets.override0 ;
            break ;

        case DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_1:
            xo_per_flow_configuration->minimum_packet_size_selection = rx_perflowsets.minpktsel1 ;
            xo_per_flow_configuration->maximum_packet_size_selection = rx_perflowsets.maxpktsel1 ;
            xo_per_flow_configuration->default_ih_class = rx_perflowsets.ihclass1 ;
            xo_per_flow_configuration->ih_class_override = rx_perflowsets.override1 ;
            break ;
        }

        return ( DRV_BBH_NO_ERROR ) ;
    }


    /* if it is one of flows 0-31: */

    /* minimum & maximum packet length selection */

    /* flows 0-15 */
    if ( xi_flow_index <= CS_RX_MAXIMAL_FLOW_INDEX_FOR_MINPKTSEL0_AND_MAXPKTSEL0_REGISTERS )
    {
        BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_READ( xi_port_index , rx_minpktsel ) ;
        BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_READ( xi_port_index , rx_maxpktsel ) ;
    }
    /* flows 16-31 */
    else
    {
        BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_READ( xi_port_index , rx_minpktsel ) ;
        BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_READ( xi_port_index , rx_maxpktsel ) ;
    }

    xo_per_flow_configuration->minimum_packet_size_selection = MS_GET_BITS( rx_minpktsel.minpktsel ,
                                                                            ( xi_flow_index * CS_RX_FIELD_LENGTH_FOR_MINPKTSEL_AND_MAXPKTSEL_REGISTERS ) % CS_NUMBER_OF_BITS_IN_REGISTER ,
                                                                            CS_RX_FIELD_LENGTH_FOR_MINPKTSEL_AND_MAXPKTSEL_REGISTERS ) ;

    xo_per_flow_configuration->maximum_packet_size_selection = MS_GET_BITS( rx_maxpktsel.maxpktsel ,
                                                                            ( xi_flow_index * CS_RX_FIELD_LENGTH_FOR_MINPKTSEL_AND_MAXPKTSEL_REGISTERS ) % CS_NUMBER_OF_BITS_IN_REGISTER ,
                                                                            CS_RX_FIELD_LENGTH_FOR_MINPKTSEL_AND_MAXPKTSEL_REGISTERS ) ;


    /* default IH class */

    /* flows 0-7 */
    if ( xi_flow_index <= CS_RX_MAXIMAL_FLOW_INDEX_FOR_IHCLASS0_REGISTER )
    {
        BBH_RX_GENERAL_CONFIGURATION_IHCLASS0_READ( xi_port_index , rx_ihclass ) ;
    }
    /* flows 8-15 */
    else if ( xi_flow_index <= CS_RX_MAXIMAL_FLOW_INDEX_FOR_IHCLASS1_REGISTER )
    {
        BBH_RX_GENERAL_CONFIGURATION_IHCLASS1_READ( xi_port_index , rx_ihclass ) ;
    }
    /* flows 16-23 */
    else if ( xi_flow_index <= CS_RX_MAXIMAL_FLOW_INDEX_FOR_IHCLASS2_REGISTER )
    {
        BBH_RX_GENERAL_CONFIGURATION_IHCLASS2_READ( xi_port_index , rx_ihclass ) ;
    }
    /* flows 24-31 */
    else
    {
        BBH_RX_GENERAL_CONFIGURATION_IHCLASS3_READ( xi_port_index , rx_ihclass ) ;
    }


    xo_per_flow_configuration->default_ih_class = MS_GET_BITS( rx_ihclass.ihclass ,
                                                               ( xi_flow_index * CS_RX_FIELD_LENGTH_FOR_IHCLASS_REGISTERS ) % CS_NUMBER_OF_BITS_IN_REGISTER ,
                                                               CS_RX_FIELD_LENGTH_FOR_IHCLASS_REGISTERS ) ;


    /* IH class override */

    BBH_RX_GENERAL_CONFIGURATION_IHOVERRIDE_READ( xi_port_index , rx_ihoverride ) ;

    xo_per_flow_configuration->ih_class_override = MS_GET_BITS( rx_ihoverride.ihoverride ,
                                                                xi_flow_index ,
                                                                1 ) ;


    return ( DRV_BBH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_bbh_rx_get_per_flow_configuration ) ;
#endif

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bbh_rx_reset                                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BBH Driver - RX Reset                                                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function enables reset of several internal units of the RX.         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_port_index - Port index                                               */
/*                                                                            */
/*   xi_units_to_reset_bitmask - bitmask of units to reset. Values of the     */
/*     enumeration DRV_BBH_RX_INTERNAL_UNIT should be ORed, as a */
/*     description of the units to be reset.                                  */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_BBH_ERROR - Return code                                       */
/*     DRV_BBH_NO_ERROR - No error                                      */
/*     DRV_BBH_INVALID_PORT_INDEX - Invalid port index                  */
/*                                                                            */
/******************************************************************************/
DRV_BBH_ERROR fi_bl_drv_bbh_rx_reset ( DRV_BBH_PORT_INDEX xi_port_index ,
                                              uint16_t xi_units_to_reset_bitmask )
{
    /* convert to 32 bit, which is needed for the HAL macro */
    uint32_t units_to_reset_bitmask = xi_units_to_reset_bitmask ;

    if ( xi_port_index >= DRV_BBH_NUMBER_OF_PORTS )
    {
        return ( DRV_BBH_INVALID_PORT_INDEX ) ;
    }

    /* the bitmask is compatible to the register format, so RX_GENERAL_CONFIGURATION_RXRSTRST struct is not used here */
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_WRITE( xi_port_index , units_to_reset_bitmask ) ;

    return ( DRV_BBH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_bbh_rx_reset ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bbh_rx_get_counters                                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BBH Driver - RX Get Counters                                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets counters of the RX. Each of these counters is cleared */
/*   when read and freezes when maximum value is reached.                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_port_index - Port index                                               */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_rx_counters - RX Counters                                             */
/*                                                                            */
/*   DRV_BBH_ERROR - Return code                                       */
/*     DRV_BBH_NO_ERROR - No error                                      */
/*     DRV_BBH_INVALID_PORT_INDEX - Invalid port index                  */
/*                                                                            */
/******************************************************************************/
DRV_BBH_ERROR fi_bl_drv_bbh_rx_get_counters ( DRV_BBH_PORT_INDEX xi_port_index ,
                                                     DRV_BBH_RX_COUNTERS * const xo_rx_counters )
{
    BBH_RX_PM_COUNTERS_INPKT rx_inpkt ;
    BBH_RX_PM_COUNTERS_TOOSHORT rx_tooshort ;
    BBH_RX_PM_COUNTERS_TOOLONG rx_toolong ;
    BBH_RX_PM_COUNTERS_CRCERROR rx_crcerror ;
    BBH_RX_PM_COUNTERS_RUNNERCONG rx_runnercong ;
    BBH_RX_PM_COUNTERS_NOBPMBN rx_nobpmbn ;
    BBH_RX_PM_COUNTERS_NOSBPMSBN rx_nosbpmsbn ;
    BBH_RX_PM_COUNTERS_NODMACD rx_nodmacd ;
    BBH_RX_PM_COUNTERS_NOSDMACD rx_nosdmacd ;

    if ( xi_port_index >= DRV_BBH_NUMBER_OF_PORTS )
    {
        return ( DRV_BBH_INVALID_PORT_INDEX ) ;
    }


    BBH_RX_PM_COUNTERS_INPKT_READ( xi_port_index , rx_inpkt ) ;
    xo_rx_counters->incoming_packets = rx_inpkt.inpkt ;

    BBH_RX_PM_COUNTERS_TOOSHORT_READ( xi_port_index , rx_tooshort ) ;
    xo_rx_counters->too_short_error = rx_tooshort.pmvalue ;

    BBH_RX_PM_COUNTERS_TOOLONG_READ( xi_port_index , rx_toolong ) ;
    xo_rx_counters->too_long_error = rx_toolong.pmvalue ;

    BBH_RX_PM_COUNTERS_CRCERROR_READ( xi_port_index , rx_crcerror ) ;
    xo_rx_counters->crc_error = rx_crcerror.pmvalue ;

    BBH_RX_PM_COUNTERS_RUNNERCONG_READ( xi_port_index , rx_runnercong ) ;
    xo_rx_counters->runner_congestion = rx_runnercong.pmvalue ;

    BBH_RX_PM_COUNTERS_NOBPMBN_READ( xi_port_index , rx_nobpmbn ) ;
    xo_rx_counters->no_bpm_bn_error = rx_nobpmbn.pmvalue ;

    BBH_RX_PM_COUNTERS_NOSBPMSBN_READ( xi_port_index , rx_nosbpmsbn ) ;
    xo_rx_counters->no_sbpm_sbn_error = rx_nosbpmsbn.pmvalue ;

    BBH_RX_PM_COUNTERS_NODMACD_READ( xi_port_index , rx_nodmacd ) ;
    xo_rx_counters->no_dma_cd_error = rx_nodmacd.pmvalue ;

    BBH_RX_PM_COUNTERS_NOSDMACD_READ( xi_port_index , rx_nosdmacd ) ;
    xo_rx_counters->no_sdma_cd_error = rx_nosdmacd.pmvalue ;

    return ( DRV_BBH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_bbh_rx_get_counters ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bbh_rx_get_iptv_filter_counter                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BBH Driver - RX Get IPTV Filter Counter                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the IPTV Filter Counter ("drop-on-miss" in IH).       */
/*   This counter is cleared when read and freezes when maximum value is      */
/*   reached.                                                                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_port_index - Port index                                               */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_iptv_filter_counter - IPTV Filter Counter                             */
/*                                                                            */
/*   DRV_BBH_ERROR - Return code                                       */
/*     DRV_BBH_NO_ERROR - No error                                      */
/*     DRV_BBH_INVALID_PORT_INDEX - Invalid port index                  */
/*                                                                            */
/******************************************************************************/
DRV_BBH_ERROR fi_bl_drv_bbh_rx_get_iptv_filter_counter ( DRV_BBH_PORT_INDEX xi_port_index ,
                                                                uint32_t * const xo_iptv_filter_counter )
{
    BBH_RX_PM_COUNTERS_IPTV rx_iptv ;

    if ( xi_port_index >= DRV_BBH_NUMBER_OF_PORTS )
    {
        return ( DRV_BBH_INVALID_PORT_INDEX ) ;
    }


    BBH_RX_PM_COUNTERS_IPTV_READ( xi_port_index , rx_iptv ) ;
    * xo_iptv_filter_counter = rx_iptv.pmvalue ;

    return ( DRV_BBH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_bbh_rx_get_iptv_filter_counter ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bbh_rx_get_error_counters                                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BBH Driver - RX Get Error Counters                                       */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets error counters of the RX. Each of these counters      */
/*   is cleared when read and freezes when maximum value is reached.          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_port_index - Port index                                               */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_rx_error_counters - RX Counters                                       */
/*                                                                            */
/*   DRV_BBH_ERROR - Return code                                       */
/*     DRV_BBH_NO_ERROR - No error                                      */
/*     DRV_BBH_INVALID_PORT_INDEX - Invalid port index                  */
/*                                                                            */
/******************************************************************************/
DRV_BBH_ERROR fi_bl_drv_bbh_rx_get_error_counters ( DRV_BBH_PORT_INDEX xi_port_index ,
                                                           DRV_BBH_RX_ERROR_COUNTERS * const xo_rx_error_counters )
{
    BBH_RX_PM_COUNTERS_SOPASOP rx_sopasop ;
    BBH_RX_PM_COUNTERS_THIRDFLOW rx_thirdflow ;
    BBH_RX_PM_COUNTERS_IHDROPPLOAM rx_ihdropploam ;
    BBH_RX_PM_COUNTERS_NOBPMBNPLOAM rx_nobpmbnploam ;
    BBH_RX_PM_COUNTERS_CRCERRORPLOAM rx_crcerrorploam ;

    if ( xi_port_index >= DRV_BBH_NUMBER_OF_PORTS )
    {
        return ( DRV_BBH_INVALID_PORT_INDEX ) ;
    }


    BBH_RX_PM_COUNTERS_SOPASOP_READ( xi_port_index , rx_sopasop ) ;
    xo_rx_error_counters->sop_after_sop_error = rx_sopasop.pmvalue ;

    BBH_RX_PM_COUNTERS_THIRDFLOW_READ( xi_port_index , rx_thirdflow ) ;
    xo_rx_error_counters->third_flow_error = rx_thirdflow.pmvalue ;

    BBH_RX_PM_COUNTERS_IHDROPPLOAM_READ( xi_port_index , rx_ihdropploam ) ;
    xo_rx_error_counters->ih_drop_error_for_ploam = rx_ihdropploam.pmvalue ;

    BBH_RX_PM_COUNTERS_NOBPMBNPLOAM_READ( xi_port_index , rx_nobpmbnploam ) ;
    xo_rx_error_counters->no_bpm_bn_error_for_ploam = rx_nobpmbnploam.pmvalue ;

    BBH_RX_PM_COUNTERS_CRCERRORPLOAM_READ( xi_port_index , rx_crcerrorploam ) ;
    xo_rx_error_counters->crc_error_for_ploam = rx_crcerrorploam.pmvalue ;

    return ( DRV_BBH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_bbh_rx_get_error_counters ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bbh_rx_get_per_flow_counters                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BBH Driver - RX Get Per Flow Counters                                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets, for the specified flow, the number of packets        */
/*   dropped by IH (due to either runner congestion or IPTV filter). These    */
/*   counters are relevant for GPON only. In Ethernet case, this information  */
/*   can be obtained using RX Get Counters API. Each of these counters is     */
/*   cleared when read and freezes when maximum value is reached. The SW      */
/*   should clear these counters in initialization stage by reading each of   */
/*   them!                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_port_index - Port index                                               */
/*                                                                            */
/*   xi_flow_index - Flow index (0-255)                                       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_number_of_packets_dropped_by_ih - Number of packets dropped by IH     */
/*                                                                            */
/*   DRV_BBH_ERROR - Return code                                       */
/*     DRV_BBH_NO_ERROR - No error                                      */
/*     DRV_BBH_INVALID_PORT_INDEX - Invalid port index                  */
/*                                                                            */
/******************************************************************************/
DRV_BBH_ERROR fi_bl_drv_bbh_rx_get_per_flow_counters(DRV_BBH_PORT_INDEX xi_port_index ,
                                                              uint8_t xi_flow_index ,
                                                              uint16_t * const xo_number_of_packets_dropped_by_ih)
{
    BBH_RX_PER_FLOW_PM_COUNTERS_PERFLOWPM rx_perflowpm ;

    if (xi_port_index != DRV_BBH_WAN)
    {
        return (DRV_BBH_API_IS_FOR_GPON_PORT_ONLY) ;
    }

    BBH_RX_PER_FLOW_PM_COUNTERS_PERFLOWPM_READ(xi_port_index , xi_flow_index , rx_perflowpm) ;
    * xo_number_of_packets_dropped_by_ih = rx_perflowpm.pmcnt ;

    return (DRV_BBH_NO_ERROR) ;
}
EXPORT_SYMBOL(fi_bl_drv_bbh_rx_get_per_flow_counters) ;

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bbh_get_gpon_bbh_fifo_clear                                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Lilac Driver BBH -  Get gpon_bbh_fifo_clear                              */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function return the bbh fifo register                               */
/*                                                                            */
/*                                                                            */
/* Registers:                                                                 */
/*                                                                            */
/*    BBH_TX_DEBUG_PDEMPTYMSB and TX_DEBUG_PDEMPTYLSB.                        */
/*                                                                            */
/* Input:                                                                     */
/*   xi_tcont_id                                                              */
/*                                                                            */
/*   xo_bbh_flush_done                                                        */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_BBH_ERROR - Return code                                       */
/*   DRV_BBH_TCONT_ID_IS_OUT_OF_RANGE - if the Tcont is out of range    */
/*                                                                            */
/******************************************************************************/
DRV_BBH_ERROR fi_bl_drv_bbh_get_bbh_fifo_clear (DRV_BBH_PORT_INDEX xi_port_index , uint8_t xi_tcont_id ,
															int32_t * const xo_bbh_flush_done )
{
	/* Local Variables */
	BBH_TX_DEBUG_PDEMPTYLSB  pd_empty_lsb ;
    BBH_TX_DEBUG_PDEMPTYMSB  pd_empty_msb ;
    uint32_t i = 1 ;
    uint32_t mask ;
    uint32_t result ;

    /* Tcont ID verification */
    if ( xi_tcont_id > CS_DRV_MAX_NUM_OF_TCONTS_PHYSICAL )
    {
	return ( DRV_BBH_TCONT_ID_IS_OUT_OF_RANGE ) ;
    }

    if (  xi_tcont_id < CS_DRV_MAX_NUM_OF_TCONTS_SW )
    {
	/* read the bbh gpon tcont flash bit */
	BBH_TX_DEBUG_PDEMPTYLSB_READ ( xi_port_index, pd_empty_lsb ) ;

	/*delay for 20 Ms*/
	mdelay( 20 ) ;

	/* build the mask */
	mask = i << xi_tcont_id ;
	result = ( pd_empty_lsb.pdempty & mask ) ;

	if ( result == 0 )
	{
		*xo_bbh_flush_done =  0 ;
	}
	else
	{
		*xo_bbh_flush_done =  1 ;
	}
    }
    else if ( xi_tcont_id <= CS_DRV_MAX_NUM_OF_TCONTS_PHYSICAL )
    {
	/* read the bbh gpon tcont flash bit */
	BBH_TX_DEBUG_PDEMPTYMSB_READ ( xi_port_index, pd_empty_msb ) ;

	/*delay for 20 Ms*/
	mdelay( 20 ) ;

	/* build the mask */
	mask = i << ( xi_tcont_id - 32 ) ;
	result = ( pd_empty_msb.pdempty & mask ) ;

	if ( result == 0 )
	{
		*xo_bbh_flush_done =  0 ;
	}
	else
	{
		*xo_bbh_flush_done =  1 ;
	}
    }
    return ( DRV_BBH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_bbh_get_bbh_fifo_clear ) ;

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bbh_get_gpon_bbh_in_segmentation                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Lilac Driver BBH -  Get gpon_bbh_is in segmentation                      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function return the bbh fifo register                               */
/*                                                                            */
/*                                                                            */
/* Registers:                                                                 */
/*                                                                            */
/*    BBH_TX_CONTEXT_SEGCNTXT.                                             */
/*                                                                            */
/* Input:                                                                     */
/*   xi_tcont_id                                                              */
/*                                                                            */
/*   xo_bbh_in_seg                                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_BBH_ERROR - Return code                                       */
/*   DRV_BBH_TCONT_ID_IS_OUT_OF_RANGE - if the Tcont is out of range    */
/*                                                                            */
/******************************************************************************/
DRV_BBH_ERROR fi_bl_drv_bbh_get_bbh_in_segmentation ( DRV_BBH_PORT_INDEX xi_port_index , uint8_t xi_tcont_id ,
																int32_t * const xo_bbh_in_seg )
{
	/* Local Variables */
	BBH_TX_CONTEXT_SEGCNTXT  segmentation_context ;

	/* Tcont ID verification */
	if ( xi_tcont_id > CS_DRV_MAX_NUM_OF_TCONTS_PHYSICAL )
	{
		return ( DRV_BBH_TCONT_ID_IS_OUT_OF_RANGE ) ;
	}

    /* Read bbh tx content */
	BBH_TX_CONTEXT_SEGCNTXT_READ (xi_port_index, xi_tcont_id ,segmentation_context) ;

    /* Check if there is segmentation in progress */
	if ( segmentation_context.inseg ==  0 )
    {
	*xo_bbh_in_seg =  0 ;
    }
    else
    {
	*xo_bbh_in_seg =  1 ;
    }
	return ( DRV_BBH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_bbh_get_bbh_in_segmentation ) ;

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bbh_set_runner_flow_ctrl_msg                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Lilac Driver BBH -  Set the runner flow control message bit.             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function Configure flow control based of Runner messages            */
/*                                                                            */
/*                                                                            */
/* Registers:                                                                 */
/*                                                                            */
/*    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL.                                  */
/*                                                                            */
/* Input:                                                                     */
/*   xi_port_index                                                            */
/*                                                                            */
/*   xi_enable                                                                */
/*                                                                            */
/* Output: none                                                               */
/*                                                                            */
/******************************************************************************/
void fi_bl_drv_bbh_set_runner_flow_ctrl_msg ( DRV_BBH_PORT_INDEX xi_port_index ,
                                                  uint32_t xi_enable )
{
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL rx_flowctrl;

    /* Configure flow control based of Runner messages.
     * Runner request to assert the flow control indication towards the Ethernet MAC. */
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_READ( xi_port_index , rx_flowctrl ) ;
    rx_flowctrl.runneren = xi_enable ;
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_WRITE( xi_port_index , rx_flowctrl ) ;

}
EXPORT_SYMBOL( fi_bl_drv_bbh_set_runner_flow_ctrl_msg ) ;


/******************************************************************************/
/*                                                                            */
/* Internal functions implementation                                          */
/*                                                                            */
/******************************************************************************/

/* this function assumes that the input paramter is valid! */
uint8_t f_convert_tx_pd_fifo_size_from_user_to_hw_format ( uint8_t xi_pd_fifo_size )
{
    return ( xi_pd_fifo_size - 1 ) ;
}

/* this function assumes that the input paramter is valid! */
uint8_t f_convert_tx_pd_fifo_size_from_hw_to_user_format ( uint8_t xi_pd_fifo_size )
{
    return ( xi_pd_fifo_size + 1 ) ;
}


int32_t f_minimum_packet_size_is_valid ( uint8_t xi_minimum_packet_size ,
                                                 uint8_t xi_packet_header_offset )
{
    if ( xi_minimum_packet_size > DRV_BBH_RX_MAXIMAL_VALUE_FOR_MINIMUM_PACKET_SIZE )
    {
        return (  0 ) ;
    }

    if ( xi_minimum_packet_size >= CS_DRV_BBH_RX_INGRESS_BUFFER_SIZE - xi_packet_header_offset )
    {
        return (  0 ) ;
    }

    return (  1 ) ;
}


int32_t f_maximum_packet_size_is_valid ( uint16_t xi_maximum_packet_size ,
                                                 uint8_t xi_packet_header_offset ,
                                                 uint8_t xi_reassembly_offset_in_8_byte )
{
    uint16_t reassembly_offset_in_byte = xi_reassembly_offset_in_8_byte * 8 ;

    if ( xi_maximum_packet_size > DRV_BBH_RX_MAXIMAL_VALUE_FOR_MAXIMUM_PACKET_SIZE )
    {
        return (  0 ) ;
    }

    if ( xi_maximum_packet_size > CS_DRV_BBH_RX_DDR_BUFFER_SIZE - xi_packet_header_offset - reassembly_offset_in_byte )
    {
        return (  0 ) ;
    }

    return (  1 ) ;
}
