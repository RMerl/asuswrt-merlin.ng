/*
   Copyright (c) 2013 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2013:DUAL/GPL:standard

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
/* This file contains the implementation of the HwApiMac for unimac           */
/*                                                                            */
/******************************************************************************/



/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/
#include "unimac_drv.h"
#if defined(_CFE_) && !defined(CONFIG_BCM96856) && !defined(CONFIG_BCM96878)
static void get_rdp_freq(uint32_t *rdp_freq) { }
#else
#include "clk_rst.h"
#endif

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
#include "rdp_drv_bbh.h"
#endif


/******************************************************************************/
/*                                                                            */
/* Types and values definitions                                               */
/*                                                                            */
/******************************************************************************/
#define UNIMAC_SWRESET_ON  			1
#define UNIMAC_SWRESET_OFF 			0
#define UNIMAC_DEFAULT_IPG 			12
#define UNIMAC_DEFAULT_PAUSE_QUANTA 0xffff
#define UNIMAC_DEFAULT_PAUSE_TIMER 	0x1ffff
#define UNIMAC_MAX_FRAME_LENGHT		0xffff

#ifdef  _CFE_    
#define  mac_unlikely(a) a      
#else
#define  mac_unlikely(a) unlikely(a)
#endif

#define UNIMAC_NUMBER_OF_SPEEDS 	( UNIMAC_SPEED_2500 + 1 )
#define UNIMAC_NUMBER_OF_DUPLEXES	( UNIMAC_DUPLEX_FULL + 1 )

static uint32_t ipgSpeedTable[UNIMAC_NUMBER_OF_SPEEDS][UNIMAC_NUMBER_OF_DUPLEXES] =
{
    {0x000a,0x000a},
    {0x000a,0x000a},
    {0x0005,0x0005},
    {0x0005,0x0005}
};

static uint32_t enabled_emac;

#if defined(CONFIG_BCM94908)
#include "bcm_map_part.h"
uint8_t *soc_base_address;      // used by DEVICE_ADDRESS()
#endif

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_configuration                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get configuration                                           */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   get the configuratin of a mac port	,note that current status of emac	  */
/*   might be different than configuration when working in autoneg			  */
/*	to get the current status use mac_hwapi_get_mac_status API				  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    emac_cfg -  structure holds the current configuration of the mac  port  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_configuration(rdpa_emac emacNum,rdpa_emac_cfg_t *emac_cfg)
{
    S_UNIMAC_CMD_REG  mCfgReg;

    if (!(enabled_emac & (1 << emacNum)))
        return;

    UNIMAC_READ32_REG(emacNum,CMD,mCfgReg);
    emac_cfg->allow_too_long		=	0;//no such
    emac_cfg->back2back_gap			=	0;//no such
    emac_cfg->check_length			=	!mCfgReg.no_lgth_check;
    emac_cfg->full_duplex			=	!mCfgReg.hd_ena;
    emac_cfg->generate_crc			=	0;
    emac_cfg->loopback				=	mCfgReg.rmt_loop_ena;
    emac_cfg->non_back2back_gap		= 	0;//no such
    emac_cfg->pad_short				= 	0;//no such
    emac_cfg->rate					=	mCfgReg.eth_speed;
    emac_cfg->rx_flow_control		=	!mCfgReg.rx_pause_ignore;
    emac_cfg->tx_flow_control		=	!mCfgReg.tx_pause_ignore;

    UNIMAC_READ_FIELD(emacNum,HD_BKP_CNTL,ipg_config_rx,emac_cfg->min_interframe_gap);
    UNIMAC_READ_FIELD(emacNum,TX_IPG_LEN,tx_ipg_len,emac_cfg->preamble_length);
}
EXPORT_SYMBOL(mac_hwapi_get_configuration);
/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_configuration                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set configuration                                           */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   set the configuratin of a mac port                                       */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*   emac_cfg -  structure holds the current configuration of the mac  port   */
/*                                                                            */
/* Output:                                                                    */
/*      																	  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_configuration(rdpa_emac emacNum,rdpa_emac_cfg_t *emac_cfg)
{
    S_UNIMAC_CMD_REG  mCfgReg;

    if (!(enabled_emac & (1 << emacNum)))
        return;

    /*first put the mac in reset to enable config change*/
    UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,1);


    UNIMAC_READ32_REG(emacNum,CMD,mCfgReg);

    /*now mac configuration can be changed */
    if(!mCfgReg.ena_ext_config) /* Change only if auto_config is not set */
    {
        mCfgReg.eth_speed           = emac_cfg->rate;
        mCfgReg.hd_ena	            = !emac_cfg->full_duplex;
        mCfgReg.rx_pause_ignore     = !emac_cfg->rx_flow_control;
        mCfgReg.tx_pause_ignore     = !emac_cfg->tx_flow_control;
        /*set the interframe gap*/
        UNIMAC_WRITE_FIELD(emacNum,HD_BKP_CNTL,ipg_config_rx,ipgSpeedTable[emac_cfg->rate][(unsigned)emac_cfg->full_duplex]);
    }
    else
    {
        /*set the interframe gap*/
        UNIMAC_WRITE_FIELD(emacNum,HD_BKP_CNTL,ipg_config_rx,emac_cfg->min_interframe_gap);
    }

    mCfgReg.no_lgth_check	      =	!emac_cfg->check_length;
    mCfgReg.pad_en                = emac_cfg->pad_short;
    mCfgReg.rmt_loop_ena	      =	emac_cfg->loopback;


    UNIMAC_WRITE32_REG(emacNum,CMD,mCfgReg);


    /* set the preamble length */
    UNIMAC_WRITE_FIELD(emacNum,TX_IPG_LEN,tx_ipg_len,emac_cfg->preamble_length);

    /* when link up at 10M, hold unimac reset longer per ASIC team */
    if (emac_cfg->rate == rdpa_emac_rate_10m)
        udelay(1000);

    /*release the sw_reset bit*/
    UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,0);
}
EXPORT_SYMBOL(mac_hwapi_set_configuration);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_duplex		                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get port duplex                                         	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	get the dulplex of a port												  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    full_duples  :  1 = full dulplex,0 = half_duplex						  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_duplex(rdpa_emac emacNum,int32_t *full_duplex)
{
    /*read field and reverse polarity*/
    int32_t fieldData;

    if (!(enabled_emac & (1 << emacNum)))
        return;

    UNIMAC_READ_FIELD(emacNum,CMD,hd_ena,fieldData);
    *full_duplex = !fieldData;
}
EXPORT_SYMBOL(mac_hwapi_get_duplex);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_duplex		                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port duplex                                         	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	set the dulplex of a port												  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    full_duples  :  1 = full dulplex,0 = half_duplex						  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_duplex(rdpa_emac emacNum,int32_t full_duplex)
{
    S_UNIMAC_CMD_REG  mCfgReg;

    if (!(enabled_emac & (1 << emacNum)))
        return;

    UNIMAC_READ32_REG(emacNum,CMD,mCfgReg);
    if (!mCfgReg.ena_ext_config) /* Change only if auto_config is not set */
    {
        /*write the dulplex field in reverse polarity,must be set in sw_reset state*/
        S_HWAPI_MAC_STATUS  macMode;

        UNIMAC_READ32_REG(emacNum,MODE,macMode);

        UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_ON);

        UNIMAC_WRITE_FIELD(emacNum,CMD,hd_ena,full_duplex ?  0 : 1);

        /*when setting the duplex we have to set new IPG value*/
        //UNIMAC_WRITE_FIELD(emacNum,TX_IPG_LEN ,tx_ipg_len,ipgSpeedTable[macMode.mac_speed][macMode.mac_duplex]);

        /*take the mac out of sw_reset*/
        UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_OFF);
    }
}
EXPORT_SYMBOL(mac_hwapi_set_duplex);




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_speed		                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get port speed rate                                      	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	get the speed of a port													  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                              							                  */
/*                                                                            */
/* Output:                                                                    */
/*      rate -   enum of the speed                                            */
/******************************************************************************/
void mac_hwapi_get_speed(rdpa_emac emacNum,rdpa_emac_rate *rate)
{
    if (!(enabled_emac & (1 << emacNum)))
        return;

    /*read the speed field*/
    UNIMAC_READ_FIELD(emacNum,CMD,eth_speed,*rate);
}
EXPORT_SYMBOL(mac_hwapi_get_speed);




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_speed		                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port speed rate                                      	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	set the speed of a port													  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*      rate -   enum of the speed   						                  */
/*                                                                            */
/* Output:                                                                    */
/*                                   							              */
/******************************************************************************/

void mac_hwapi_set_speed(rdpa_emac emacNum,rdpa_emac_rate rate)
{
    S_UNIMAC_CMD_REG  mCfgReg;

    if (!(enabled_emac & (1 << emacNum)))
        return;

    UNIMAC_READ32_REG(emacNum,CMD,mCfgReg);
    if (!mCfgReg.ena_ext_config) /* Change only if auto_config is not set */
    {
        UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_ON);

        UNIMAC_WRITE_FIELD(emacNum,CMD,eth_speed,rate);


        UNIMAC_WRITE_FIELD(emacNum,HD_BKP_CNTL,ipg_config_rx,ipgSpeedTable[rate][0]);


        UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_OFF);
    }
}
EXPORT_SYMBOL(mac_hwapi_set_speed);

void mac_hwapi_set_external_conf(rdpa_emac emacNum, int enable)
{
    S_UNIMAC_CMD_REG  mCfgReg;

    if (!(enabled_emac & (1 << emacNum)))
        return;

    UNIMAC_READ32_REG(emacNum,CMD,mCfgReg);
    mCfgReg.ena_ext_config = enable;
    UNIMAC_WRITE32_REG(emacNum,CMD,mCfgReg);
}
EXPORT_SYMBOL(mac_hwapi_set_external_conf);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_rxtx_enable                                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set tx and rx enable                                  	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	get tx and rx enable													  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                                             								  */
/*                                                                            */
/* Output:                                                                    */
/*       rxtxEnable  - boolean enable   	                                  */
/******************************************************************************/
void mac_hwapi_get_rxtx_enable(rdpa_emac emacNum,int32_t *rxEnable,int32_t *txEnable)
{
    if (!(enabled_emac & (1 << emacNum)))
        return;

    UNIMAC_READ_FIELD(emacNum,CMD,tx_ena,*txEnable);
    UNIMAC_READ_FIELD(emacNum,CMD,rx_ena,*rxEnable);
}
EXPORT_SYMBOL(mac_hwapi_get_rxtx_enable);





/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_rxtx_enable                                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set tx and rx enable                                  	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	set tx and rx enable													  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*    rxtxEnable  - boolean enable                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_rxtx_enable(rdpa_emac emacNum,int32_t rxEnable,int32_t txEnable)
{
    if (!(enabled_emac & (1 << emacNum)))
        return;

    UNIMAC_WRITE_FIELD(emacNum,CMD,tx_ena,txEnable);
    UNIMAC_WRITE_FIELD(emacNum,CMD,rx_ena,rxEnable);
}
EXPORT_SYMBOL(mac_hwapi_set_rxtx_enable);




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_sw_reset		                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get port software reset                                  	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	get the sw reset bit of emac port										  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    swReset  :  1 = reset,0 = not reset									  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_sw_reset(rdpa_emac emacNum,int32_t *swReset)
{
    if (!(enabled_emac & (1 << emacNum)))
        return;

    UNIMAC_READ_FIELD(emacNum,CMD,sw_reset,*swReset);
}
EXPORT_SYMBOL(mac_hwapi_get_sw_reset);




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_sw_reset		                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port software reset                                  	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	set the sw reset bit of emac port										  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*   swReset  :  1 = reset,0 = not reset	                                  */
/*                                                                            */
/* Output:                                                                    */
/*    								  										  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_sw_reset(rdpa_emac emacNum,int32_t swReset)
{
    if (!(enabled_emac & (1 << emacNum)))
        return;

    UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,swReset);
}
EXPORT_SYMBOL(mac_hwapi_set_sw_reset);


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_tx_min_pkt_size                                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get tx minimum packet size                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  Get the unimac configuration for minimum tx packet size                   */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    min_pkt_size  :  14...125                                               */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_tx_min_pkt_size(rdpa_emac emacNum,int32_t *min_pkt_size)
{
    if (!(enabled_emac & (1 << emacNum)))
        return;

    UNIMAC_READ_FIELD(emacNum,TX_IPG_LEN,tx_min_pkt_size,*min_pkt_size);
}
EXPORT_SYMBOL(mac_hwapi_get_tx_min_pkt_size);




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_tx_min_pkt_size                                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set tx minimum packet size                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  Set the unimac configuration for minimum tx packet size                   */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*   min_pkt_size  :  14...125                                                */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_tx_min_pkt_size(rdpa_emac emacNum,int32_t min_pkt_size)
{
    if (!(enabled_emac & (1 << emacNum)))
        return;

#if defined(CONFIG_BCM94908) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM947622)
    UNIMAC_WRITE_FIELD(emacNum,TX_IPG_LEN,tx_min_pkt_size,min_pkt_size);
#endif
}
EXPORT_SYMBOL(mac_hwapi_set_tx_min_pkt_size);


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_tx_max_frame_len                                           */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get port TX MTU                                         	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	get the port maximum transmit unit size in bytes						  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                              										      */
/*                                                                            */
/* Output:                                                                    */
/*    maxTxFrameLen - size of frame in bytes 								  */
/*                                                                            */
/******************************************************************************/
#if defined(CONFIG_BCM947622)
void mac_hwapi_get_tx_max_frame_len(rdpa_emac emacNum,uint32_t *maxTxFrameLen )
{
    S_UNIMAC_TOPCTRL_MAX_PKT_SZ_REG cfgreg;
    uintptr_t address = (uintptr_t)UNIMAC_TOPCTRL_MIB_MAX_PKT_SIZE + UNIMAC_TOPCTRL_INSTANCE_OFFSET(emacNum);

    if (!(enabled_emac & (1 << emacNum)))
        return;

    READ_32(address,cfgreg);

    *maxTxFrameLen = cfgreg.max_pkt_size;
}
#else //!47622
void mac_hwapi_get_tx_max_frame_len(rdpa_emac emacNum,uint32_t *maxTxFrameLen )
{
    S_UNIMAC_TOP_CFG1_REG cfgreg;
    uintptr_t address = (uintptr_t)UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1 + UNIMAC_MISC_INSTANCE_OFFSET(emacNum);

    if (!(enabled_emac & (1 << emacNum)))
        return;

    READ_32(address,cfgreg);

    *maxTxFrameLen = cfgreg.max_pkt_size;
}
#endif //!47622
EXPORT_SYMBOL(mac_hwapi_get_tx_max_frame_len);




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_tx_max_frame_len		                                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port TX MTU                                         	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	set the port maximum transmit unit size in bytes						  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*    maxTxFrameLen - size of frame in bytes                         	      */
/*                                                                            */
/* Output:                                                                    */
/*   																		  */
/*                                                                            */
/******************************************************************************/
#if defined(CONFIG_BCM947622)
void mac_hwapi_set_tx_max_frame_len(rdpa_emac emacNum,uint32_t maxTxFrameLen )
{

    S_UNIMAC_TOPCTRL_MAX_PKT_SZ_REG cfgreg;
    uintptr_t address = (uintptr_t)UNIMAC_TOPCTRL_MIB_MAX_PKT_SIZE + UNIMAC_TOPCTRL_INSTANCE_OFFSET(emacNum);

    if (!(enabled_emac & (1 << emacNum)))
        return;

    /*put emac in sw reset state*/
    UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_ON );

    READ_32(address,cfgreg);
    cfgreg.max_pkt_size = maxTxFrameLen;
    WRITE_32(address,cfgreg);

    UNIMAC_WRITE_FIELD(emacNum,FRM_LEN,frame_length,UNIMAC_MAX_FRAME_LENGHT);

    UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_OFF );
}
#else //!47622
void mac_hwapi_set_tx_max_frame_len(rdpa_emac emacNum,uint32_t maxTxFrameLen )
{

    S_UNIMAC_TOP_CFG1_REG cfgreg;
    uintptr_t address = (uintptr_t)UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1 + UNIMAC_MISC_INSTANCE_OFFSET(emacNum);

    if (!(enabled_emac & (1 << emacNum)))
        return;

    /*put emac in sw reset state*/
    UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_ON );

    READ_32(address,cfgreg);
    cfgreg.max_pkt_size = maxTxFrameLen;
    WRITE_32(address,cfgreg);

    UNIMAC_WRITE_FIELD(emacNum,FRM_LEN,frame_length,UNIMAC_MAX_FRAME_LENGHT);

    UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_OFF );
}
#endif //!47622
EXPORT_SYMBOL(mac_hwapi_set_tx_max_frame_len);



/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_rx_max_frame_len		                                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get port RX MTU                                         	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	get the port maximum receive unit size in bytes							  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                           	     										  */
/*                                                                            */
/* Output:                                                                    */
/*   maxRxFrameLen - size of current MRU									  */
/*                                                                            */
/******************************************************************************/
#if defined(CONFIG_BCM947622)
void mac_hwapi_get_rx_max_frame_len(rdpa_emac emacNum,uint32_t *maxRxFrameLen )
{
    if (!(enabled_emac & (1 << emacNum)))
        return;

    UNIMAC_READ_FIELD(emacNum,FRM_LEN,frame_length,*maxRxFrameLen );
}
#else //!47622
void mac_hwapi_get_rx_max_frame_len(rdpa_emac emacNum,uint32_t *maxRxFrameLen )
{
    if (!(enabled_emac & (1 << emacNum)))
        return;

    UNIMAC_READ_FIELD(emacNum,RX_MAX_PKT_SIZE,max_pkt_size,*maxRxFrameLen );
}
#endif //!47622
EXPORT_SYMBOL(mac_hwapi_get_rx_max_frame_len);



/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_rx_max_frame_len		                                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port RX MTU                                         	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	set the port maximum receive unit size in bytes							  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*    maxRxFrameLen - size of current MRU        							  */
/*                                                                            */
/* Output:                                                                    */
/*   									  									  */
/*                                                                            */
/******************************************************************************/
#if defined(CONFIG_BCM947622)
void mac_hwapi_set_rx_max_frame_len(rdpa_emac emacNum,uint32_t maxRxFrameLen )
{
    S_UNIMAC_TOPCTRL_MAX_PKT_SZ_REG cfgreg;
    uintptr_t address = (uintptr_t)UNIMAC_TOPCTRL_MIB_MAX_PKT_SIZE + UNIMAC_TOPCTRL_INSTANCE_OFFSET(emacNum);

    if (!(enabled_emac & (1 << emacNum)))
        return;

    /*put emac in sw reset state*/
    UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_ON );

    READ_32(address,cfgreg);
    cfgreg.max_pkt_size = maxRxFrameLen;
    WRITE_32(address,cfgreg);

    UNIMAC_WRITE_FIELD(emacNum,FRM_LEN,frame_length,maxRxFrameLen);

    UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_OFF );
}
#else //!47622
void mac_hwapi_set_rx_max_frame_len(rdpa_emac emacNum,uint32_t maxRxFrameLen )
{
    S_UNIMAC_TOP_CFG1_REG cfgreg;
    uintptr_t address = (uintptr_t)UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1 + UNIMAC_MISC_INSTANCE_OFFSET(emacNum);

    if (!(enabled_emac & (1 << emacNum)))
        return;

    /*put emac in sw reset state*/
    UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_ON );

    READ_32(address,cfgreg);
    cfgreg.max_pkt_size = maxRxFrameLen;
    WRITE_32(address,cfgreg);

    UNIMAC_WRITE_FIELD(emacNum,RX_MAX_PKT_SIZE,max_pkt_size,maxRxFrameLen);

    /*put emac out sw reset state*/
    UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_OFF );
}
#endif //!47622
EXPORT_SYMBOL(mac_hwapi_set_rx_max_frame_len);



/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_tx_igp_len	                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port inter frame gap                                 	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	set the inter frame gap	size in bytes									  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*   txIpgLen - length in bytes                                               */
/*                                                                            */
/* Output:                                                                    */
/*    																		  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_tx_igp_len(rdpa_emac emacNum,uint32_t txIpgLen )
{
    /*we assume that txIpgLen called with bits time resolution*/
    uint32_t	newIpg;

    if (!(enabled_emac & (1 << emacNum)))
        return;

    /*put emac in sw reset state*/
    UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_ON );

    txIpgLen += 7;
    newIpg = txIpgLen / 8;

    if (newIpg < 8)
    {
        newIpg = 8;
    } else if (newIpg > 27)
    {
        newIpg = 27;
    }
    UNIMAC_WRITE_FIELD(emacNum,TX_IPG_LEN,tx_ipg_len,newIpg );

    /*put emac out sw reset state*/
    UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_OFF );

}
EXPORT_SYMBOL(mac_hwapi_set_tx_igp_len);



/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_tx_igp_len	                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get port inter frame gap                                 	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	get the inter frame gap	size in bytes									  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                                      							          */
/*                                                                            */
/* Output:                                                                    */
/*    txIpgLen - length in bytes    										  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_tx_igp_len(rdpa_emac emacNum,uint32_t *txIpgLen )
{
    if (!(enabled_emac & (1 << emacNum)))
        return;

    UNIMAC_READ_FIELD(emacNum,TX_IPG_LEN,tx_ipg_len,*txIpgLen );

}
EXPORT_SYMBOL(mac_hwapi_get_tx_igp_len);




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_mac_status	                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port status                                         	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	set the status of mac													  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    macStatus  :  														  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_mac_status(rdpa_emac emacNum,S_HWAPI_MAC_STATUS *macStatus)
{

}
EXPORT_SYMBOL(mac_hwapi_get_mac_status);





/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_flow_control                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get port flow control                                    	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	get the flow control of a port											  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*           																  */
/*                                                                            */
/* Output:                                                                    */
/*   flowControl - structure with parameters of tx and rx flow control		  */
/*                                                                            */
/******************************************************************************/
#if defined(CONFIG_BCM947622)
void mac_hwapi_get_flow_control(rdpa_emac emacNum,S_MAC_HWAPI_FLOW_CTRL *flowControl)
{
    uint32_t	rxFlow, txFlow;

    if (!(enabled_emac & (1 << emacNum)))
        return;

    UNIMAC_READ_FIELD(emacNum,CMD,rx_pause_ignore,rxFlow);
    UNIMAC_READ_FIELD(emacNum,CMD,tx_pause_ignore,txFlow);
    flowControl->rxFlowEnable	= !rxFlow;
    flowControl->txFlowEnable	= !txFlow;
}
#else //!47622
void mac_hwapi_get_flow_control(rdpa_emac emacNum,S_MAC_HWAPI_FLOW_CTRL *flowControl)
{
    uint32_t	rxFlow, txFlow, value;
    uintptr_t	misc_top_address;

    if (!(enabled_emac & (1 << emacNum)))
        return;

    misc_top_address = (uintptr_t)UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2 + UNIMAC_MISC_INSTANCE_OFFSET(emacNum);
    READ_32(misc_top_address, value);


    if(value & UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_INT)
    {
        UNIMAC_READ_FIELD(emacNum,CMD,rx_pause_ignore,rxFlow);
        UNIMAC_READ_FIELD(emacNum,CMD,tx_pause_ignore,txFlow);
        flowControl->rxFlowEnable	= !rxFlow;
        flowControl->txFlowEnable	= !txFlow;
    }
    else
    {
        flowControl->rxFlowEnable = 0;
        flowControl->txFlowEnable = 0;
    }
}
#endif //!47622
EXPORT_SYMBOL(mac_hwapi_get_flow_control);
/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_pause_params                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port flow control                                    	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	set the flow control of a port											  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*   flowControl - structure with parameters of tx and rx flow control        */
/*                                                                            */
/* Output:                                                                    */
/*   						  												  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_pause_params(rdpa_emac emacNum,int32_t pauseCtrlEnable,uint32_t pauseTimer,uint32_t pauseQuanta)
{
    if (!(enabled_emac & (1 << emacNum)))
        return;

    UNIMAC_WRITE_FIELD(emacNum,PAUSE_CNTRL,pause_control_en,pauseCtrlEnable );
    UNIMAC_WRITE_FIELD(emacNum,PAUSE_CNTRL,pause_timer,pauseTimer );
    UNIMAC_WRITE_FIELD(emacNum,PAUSE_QUNAT,pause_quant,pauseQuanta );
}
EXPORT_SYMBOL(mac_hwapi_set_pause_params);


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_flow_control                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port flow control                                    	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	set the flow control of a port											  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*   flowControl - structure with parameters of tx and rx flow control        */
/*                                                                            */
/* Output:                                                                    */
/*   						  												  */
/*                                                                            */
/******************************************************************************/
#if defined(CONFIG_BCM947622)
void mac_hwapi_set_flow_control(rdpa_emac emacNum,S_MAC_HWAPI_FLOW_CTRL *flowControl)
{
    if (!(enabled_emac & (1 << emacNum)))
        return;

    UNIMAC_WRITE_FIELD(emacNum,CMD,sw_override_rx, 1);
    UNIMAC_WRITE_FIELD(emacNum,CMD,sw_override_tx, 1);

    UNIMAC_WRITE_FIELD(emacNum,CMD,rx_pause_ignore, !flowControl->rxFlowEnable);
    UNIMAC_WRITE_FIELD(emacNum,CMD,tx_pause_ignore, !flowControl->txFlowEnable);
}
#else //!47622
void mac_hwapi_set_flow_control(rdpa_emac emacNum,S_MAC_HWAPI_FLOW_CTRL *flowControl)
{
    uintptr_t misc_top_address;
    uint32_t value;

    if (!(enabled_emac & (1 << emacNum)))
        return;

    UNIMAC_WRITE_FIELD(emacNum,CMD,rx_pause_ignore, !flowControl->rxFlowEnable);
    UNIMAC_WRITE_FIELD(emacNum,CMD,tx_pause_ignore, !flowControl->txFlowEnable);
    misc_top_address = (uintptr_t)UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2 + UNIMAC_MISC_INSTANCE_OFFSET(emacNum);
    READ_32(misc_top_address, value);

    if(flowControl->rxFlowEnable || flowControl->txFlowEnable)
        value |= UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_INT;
    else
        value &= ~UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_INT;

    WRITE_32(misc_top_address, value);
}
#endif //!47622
EXPORT_SYMBOL(mac_hwapi_set_flow_control);


static rdpa_emac_rx_stat_t rxCountersLast[MAX_NUM_OF_EMACS];
#define UNIMAC_CURRENT_RX_COUNTERS(emacNum, rxCounters, param) \
    do{\
        uint32_t  paramCurrent = rxCounters->param; \
        if(mac_unlikely(rxCounters->param < rxCountersLast[emacNum].param)){    \
            rxCounters->param += (0xFFFFFFFF-rxCountersLast[emacNum].param)+1;    \
        }else{  \
            rxCounters->param -= rxCountersLast[emacNum].param; \
        }   \
        rxCountersLast[emacNum].param   = paramCurrent; \
    }while(0)

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_rx_counters	                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - 	get the rx counters of port                           	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	get the rx counters of port				  								  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    rxCounters  :  structure filled with counters							  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_rx_counters(rdpa_emac emacNum,rdpa_emac_rx_stat_t *rxCounters)
{
    uint32_t		tempVal;

    if (!(enabled_emac & (1 << emacNum)))
    {
        memset(rxCounters, 0, sizeof(rdpa_emac_rx_stat_t));
        return;
    }

    UNIMAC_READ32_MIB(emacNum,GRALN,rxCounters->alignment_error);
    UNIMAC_READ32_MIB(emacNum,GRBCA,rxCounters->broadcast_packet);
    UNIMAC_READ32_MIB(emacNum,GRUC, rxCounters->unicast_packet);
    UNIMAC_READ32_MIB(emacNum,GRBYT,rxCounters->byte);
    UNIMAC_READ32_MIB(emacNum,RRBYT,tempVal);
    rxCounters->byte                    +=	tempVal;
    UNIMAC_READ32_MIB(emacNum,GRFCR,rxCounters->carrier_sense_error);
    UNIMAC_READ32_MIB(emacNum,GRCDE,rxCounters->code_error);
    UNIMAC_READ32_MIB(emacNum,GRXCF,rxCounters->control_frame);
    UNIMAC_READ32_MIB(emacNum,GRFCS,rxCounters->fcs_error);
    rxCounters->fragments				=	0;//?????
    UNIMAC_READ32_MIB(emacNum,GR255,rxCounters->frame_128_255);
    UNIMAC_READ32_MIB(emacNum,GR2047,tempVal);
    rxCounters->frame_1519_mtu			=	tempVal;
    UNIMAC_READ32_MIB(emacNum,GR4095,tempVal);
    rxCounters->frame_1519_mtu			+=	tempVal;
    UNIMAC_READ32_MIB(emacNum,GR9216,tempVal);
    rxCounters->frame_1519_mtu			+=	tempVal;

    UNIMAC_READ32_MIB(emacNum,GR511,rxCounters->frame_256_511);

    UNIMAC_READ32_MIB(emacNum,GR1023,rxCounters->frame_512_1023);

    UNIMAC_READ32_MIB(emacNum,GR1518,rxCounters->frame_1024_1518);

    UNIMAC_READ32_MIB(emacNum,GR64,rxCounters->frame_64);

    UNIMAC_READ32_MIB(emacNum,GR127,rxCounters->frame_65_127);

    UNIMAC_READ32_MIB(emacNum,GRFLR,rxCounters->frame_length_error);

    UNIMAC_READ32_MIB(emacNum,GRJBR,rxCounters->jabber);

    UNIMAC_READ32_MIB(emacNum,GRMCA,rxCounters->multicast_packet);

    rxCounters->overflow				=	0;//no matched counter
    UNIMAC_READ32_MIB(emacNum,GROVR,rxCounters->oversize_packet);
	
    UNIMAC_READ32_MIB(emacNum,RRPKT,rxCounters->undersize_packet);

    UNIMAC_READ32_MIB(emacNum,GRPKT,rxCounters->packet);
    
    UNIMAC_READ32_MIB(emacNum,GRXPF,rxCounters->pause_control_frame);

    UNIMAC_READ32_MIB(emacNum,GRXUO,rxCounters->unknown_opcode);

    UNIMAC_CURRENT_RX_COUNTERS(emacNum, rxCounters, alignment_error);
    UNIMAC_CURRENT_RX_COUNTERS(emacNum, rxCounters, broadcast_packet);
    UNIMAC_CURRENT_RX_COUNTERS(emacNum, rxCounters, unicast_packet);
    UNIMAC_CURRENT_RX_COUNTERS(emacNum, rxCounters, byte);
    UNIMAC_CURRENT_RX_COUNTERS(emacNum, rxCounters, carrier_sense_error);
    UNIMAC_CURRENT_RX_COUNTERS(emacNum, rxCounters, code_error);
    UNIMAC_CURRENT_RX_COUNTERS(emacNum, rxCounters, control_frame);
    UNIMAC_CURRENT_RX_COUNTERS(emacNum, rxCounters, fcs_error);
    UNIMAC_CURRENT_RX_COUNTERS(emacNum, rxCounters, frame_64);
    UNIMAC_CURRENT_RX_COUNTERS(emacNum, rxCounters, frame_65_127);
    UNIMAC_CURRENT_RX_COUNTERS(emacNum, rxCounters, frame_128_255);
    UNIMAC_CURRENT_RX_COUNTERS(emacNum, rxCounters, frame_256_511);
    UNIMAC_CURRENT_RX_COUNTERS(emacNum, rxCounters, frame_512_1023);
    UNIMAC_CURRENT_RX_COUNTERS(emacNum, rxCounters, frame_1024_1518);
    UNIMAC_CURRENT_RX_COUNTERS(emacNum, rxCounters, frame_1519_mtu);
    UNIMAC_CURRENT_RX_COUNTERS(emacNum, rxCounters, frame_length_error);
    UNIMAC_CURRENT_RX_COUNTERS(emacNum, rxCounters, jabber);
    UNIMAC_CURRENT_RX_COUNTERS(emacNum, rxCounters, multicast_packet);
    UNIMAC_CURRENT_RX_COUNTERS(emacNum, rxCounters, oversize_packet);
    UNIMAC_CURRENT_RX_COUNTERS(emacNum, rxCounters, undersize_packet);
    UNIMAC_CURRENT_RX_COUNTERS(emacNum, rxCounters, packet);
    UNIMAC_CURRENT_RX_COUNTERS(emacNum, rxCounters, pause_control_frame);
    UNIMAC_CURRENT_RX_COUNTERS(emacNum, rxCounters, unknown_opcode);
}
EXPORT_SYMBOL(mac_hwapi_get_rx_counters);


static rdpa_emac_tx_stat_t txCountersLast[MAX_NUM_OF_EMACS];
#define UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, param) \
    do{\
        uint32_t  paramCurrent = txCounters->param; \
        if(mac_unlikely(txCounters->param < txCountersLast[emacNum].param)){    \
            txCounters->param += (0xFFFFFFFF-txCountersLast[emacNum].param)+1;    \
        }else{  \
            txCounters->param -= txCountersLast[emacNum].param; \
        }   \
        txCountersLast[emacNum].param   = paramCurrent; \
    }while(0)


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_tx_counters	                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - 	get the tx counters of port                           	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	get the tx counters of port				  								  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    txCounters  :  structure filled with counters							  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_tx_counters(rdpa_emac emacNum,rdpa_emac_tx_stat_t *txCounters)
{
    /*fill the rdpa counters structure*/
    uint32_t		tempVal;

    if (!(enabled_emac & (1 << emacNum)))
    {
        memset(txCounters, 0, sizeof(rdpa_emac_tx_stat_t));
        return;
    }

    UNIMAC_READ32_MIB(emacNum,GTBYT,txCounters->byte);

    UNIMAC_READ32_MIB(emacNum,GTBCA,txCounters->broadcast_packet);

    UNIMAC_READ32_MIB(emacNum,GTMCA, txCounters->multicast_packet);

    UNIMAC_READ32_MIB(emacNum,GTUC, txCounters->unicast_packet);

    UNIMAC_READ32_MIB(emacNum,GTXCF,txCounters->control_frame);

    UNIMAC_READ32_MIB(emacNum,GTDRF,txCounters->deferral_packet);


    UNIMAC_READ32_MIB(emacNum,GTNCL,tempVal);
    txCounters->error						= tempVal;
    UNIMAC_READ32_MIB(emacNum,GTOVR,tempVal);
    txCounters->error						+= tempVal;
    UNIMAC_READ32_MIB(emacNum,GTFCS,tempVal);
    txCounters->error						+= tempVal;
    UNIMAC_READ32_MIB(emacNum,GTEDF,txCounters->excessive_deferral_packet);

    UNIMAC_READ32_MIB(emacNum,GTFCS,txCounters->fcs_error	);

    UNIMAC_READ32_MIB(emacNum,GTFRG,txCounters->fragments_frame);

    UNIMAC_READ32_MIB(emacNum,TR1518,txCounters->frame_1024_1518);

    UNIMAC_READ32_MIB(emacNum,TR255,  txCounters->frame_128_255);

    UNIMAC_READ32_MIB(emacNum,TR2047,  tempVal);
    txCounters->frame_1519_mtu			=   tempVal;
    UNIMAC_READ32_MIB(emacNum,TR4095,  tempVal);
    txCounters->frame_1519_mtu			+=   tempVal;
    UNIMAC_READ32_MIB(emacNum,TR9216,  tempVal);
    txCounters->frame_1519_mtu			+=   tempVal;
    UNIMAC_READ32_MIB(emacNum,TR511,  txCounters->frame_256_511);

    UNIMAC_READ32_MIB(emacNum,TR1023, txCounters->frame_512_1023);

    UNIMAC_READ32_MIB(emacNum,TR64,txCounters->frame_64);

    UNIMAC_READ32_MIB(emacNum,TR127, txCounters->frame_65_127);

    UNIMAC_READ32_MIB(emacNum,GTJBR, txCounters->jabber_frame);

    UNIMAC_READ32_MIB(emacNum,GTLCL, txCounters->late_collision);

    UNIMAC_READ32_MIB(emacNum,GTMCL, txCounters->multiple_collision);

    UNIMAC_READ32_MIB(emacNum,GTOVR,txCounters->oversize_frame);

    UNIMAC_READ32_MIB(emacNum,GTPOK, txCounters->packet);

    UNIMAC_READ32_MIB(emacNum,GTXPF, txCounters->pause_control_frame);

    UNIMAC_READ32_MIB(emacNum,GTSCL, txCounters->single_collision);

    UNIMAC_READ32_MIB(emacNum,GTNCL, txCounters->total_collision);

    UNIMAC_READ32_MIB(emacNum,GTXCL, txCounters->excessive_collision);

    txCounters->underrun					=	0;
    txCounters->undersize_frame				=	0;

    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, byte);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, packet);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, broadcast_packet);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, multicast_packet);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, unicast_packet);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, deferral_packet);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, excessive_deferral_packet);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, control_frame);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, jabber_frame);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, oversize_frame);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, pause_control_frame);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, fragments_frame);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, error);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, fcs_error);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, frame_64);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, frame_65_127);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, frame_128_255);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, frame_256_511);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, frame_512_1023);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, frame_1024_1518);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, frame_1519_mtu);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, single_collision);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, total_collision);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, late_collision);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, multiple_collision);
    UNIMAC_CURRENT_TX_COUNTERS(emacNum, txCounters, excessive_collision);
}
EXPORT_SYMBOL(mac_hwapi_get_tx_counters);
/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_init_emac	                         	                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - 	init the emac to well known state                      	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	initialized the emac port				  								  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_init_emac(rdpa_emac emacNum)
{
    /*configure the emac and put it in automatic speed mode*/
    S_UNIMAC_CMD_REG	mCfgReg;
    uint32_t max_frame_length = UNIMAC_MAX_FRAME_LENGHT;

#if defined(CONFIG_BCM94908)
    soc_base_address = RDP_BASE;
#endif

    /*before calling this function make sure you pull out of reset the emac */

    /*put the emac in sw_reset state*/
    //UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_ON);

    UNIMAC_READ32_REG(emacNum,CMD,mCfgReg);
    /* Do the initialization */
    mCfgReg.tx_ena			=	0;
    mCfgReg.rx_ena			=	0;
    /* for 63138 - even though the EMAC_1 is connected to SF2 @ 2Gbps, we still set the link to 1G.
     * Actual speed of the link is derived from SF2 based on the speed set for IMP port#8.  */
    mCfgReg.eth_speed		=	UNIMAC_SPEED_1000;
    mCfgReg.promis_en		=	1;
    mCfgReg.pad_en			=	0;
    mCfgReg.pause_fwd		=	1;
    mCfgReg.crc_fwd			=	1;
    mCfgReg.ena_ext_config	=	0;
    mCfgReg.rx_pause_ignore	=	0;
    mCfgReg.tx_pause_ignore	=	0;
    mCfgReg.tx_addr_ins		=	0;
    mCfgReg.lcl_loop_ena	=	0;
    mCfgReg.cntl_frm_ena	=	1;
    mCfgReg.no_lgth_check	=	1;
    mCfgReg.rmt_loop_ena	=	0;
    mCfgReg.rx_err_disc		=	0;
    mCfgReg.prbl_ena		=	0;
    mCfgReg.cntl_frm_ena	=	1;

    /*write the configuration */
    UNIMAC_WRITE32_REG(emacNum,CMD,mCfgReg);
    UNIMAC_WRITE32_REG(emacNum,FRM_LEN, max_frame_length);

    /*take out the emac from sw_reset state*/
    //UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_OFF);
#if 0
    /*configure the pause control*/
    UNIMAC_WRITE_FIELD(emacNum,PAUSE_CNTRL,pause_timer,UNIMAC_DEFAULT_PAUSE_TIMER);
    UNIMAC_WRITE_FIELD(emacNum,PAUSE_CNTRL,pause_control_en,1);

    /*configure the pause quanta*/
    UNIMAC_WRITE_FIELD(emacNum,PAUSE_QUNAT,pause_quant,UNIMAC_DEFAULT_PAUSE_QUANTA);

    /*write default ipg*/
    UNIMAC_WRITE_FIELD(emacNum,TX_IPG_LEN,tx_ipg_len,UNIMAC_DEFAULT_IPG);
#endif
    memset(&rxCountersLast, 0, MAX_NUM_OF_EMACS*sizeof(rdpa_emac_rx_stat_t));
    memset(&txCountersLast, 0, MAX_NUM_OF_EMACS*sizeof(rdpa_emac_tx_stat_t));

    enabled_emac |= (1 << emacNum);

    /*emac is ready to go!*/
}
EXPORT_SYMBOL(mac_hwapi_init_emac);
/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_loopback	                         	                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - 	init the emac to well known state                      	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	initialized the emac port				  								  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_loopback(rdpa_emac emacNum,MAC_LPBK *loopback)
{
    S_UNIMAC_CMD_REG	mCmdReg;

    if (!(enabled_emac & (1 << emacNum)))
        return;

    UNIMAC_READ32_REG(emacNum,CMD,mCmdReg);

    if (mCmdReg.lcl_loop_ena && mCmdReg.rmt_loop_ena)
    {
        *loopback = MAC_LPBK_BOTH;
    }
    else if (mCmdReg.lcl_loop_ena)
    {
        *loopback = MAC_LPBK_LOCAL;
    }
    else if (mCmdReg.rmt_loop_ena)
    {
        *loopback = MAC_LPBK_REMOTE;
    }
    else
    {
        *loopback = MAC_LPBK_NONE;
    }

}
EXPORT_SYMBOL(mac_hwapi_get_loopback);


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_loopback	                         	                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - 	set loopback type of the mac	                      	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	initialized the emac port				  								  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_loopback(rdpa_emac emacNum,MAC_LPBK loopback)
{
    if (!(enabled_emac & (1 << emacNum)))
        return;

    /*put the emac in sw_reset state*/
    UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_ON);

    switch(loopback)
    {
        case MAC_LPBK_BOTH:
            UNIMAC_WRITE_FIELD(emacNum,CMD,rmt_loop_ena,1);
            UNIMAC_WRITE_FIELD(emacNum,CMD, lcl_loop_ena,1);
            break;
        case MAC_LPBK_LOCAL:
            UNIMAC_WRITE_FIELD(emacNum,CMD, lcl_loop_ena,1);
            break;
        case MAC_LPBK_REMOTE:
            UNIMAC_WRITE_FIELD(emacNum,CMD,rmt_loop_ena,1);
            break;
        case MAC_LPBK_NONE:
            UNIMAC_WRITE_FIELD(emacNum,CMD,rmt_loop_ena,0);
            UNIMAC_WRITE_FIELD(emacNum,CMD, lcl_loop_ena,0);
            break;
        default:

            break;
    }
    /* take out the emac from sw_reset state*/
    UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_OFF);

}
EXPORT_SYMBOL(mac_hwapi_set_loopback);

#if !defined(CONFIG_BCM947622)
void mac_hwapi_set_unimac_cfg(rdpa_emac emacNum, int32_t enabled)
{
    S_UNIMAC_CFG_REG cfgreg;
    uintptr_t address = (uintptr_t)UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG + UNIMAC_MISC_INSTANCE_OFFSET(emacNum);

    if (!(enabled_emac & (1 << emacNum)))
        return;

    READ_32(address,cfgreg);
    cfgreg.gmii_direct = enabled ? 1 : 0;
    WRITE_32(address,cfgreg);
}
EXPORT_SYMBOL(mac_hwapi_set_unimac_cfg);
#endif //!47622

void mac_hwapi_modify_flow_control_pause_pkt_addr ( rdpa_emac emacNum,bdmf_mac_t mac)
{
    uint32_t value = *(uint32_t*)(&(mac.b[4])) >> 16 ;
    uintptr_t mac_address = (uintptr_t)UNIMAC_CONFIGURATION_UMAC_0_RDP_MAC0 + UNIMAC_CONF_INSTANCE_OFFSET(emacNum);

    if (!(enabled_emac & (1 << emacNum)))
        return;

    WRITE_32(mac_address, *(uint32_t*)mac.b);

    mac_address = (uintptr_t)UNIMAC_CONFIGURATION_UMAC_0_RDP_MAC1 + UNIMAC_CONF_INSTANCE_OFFSET(emacNum);
    WRITE_32(mac_address, value);

}
EXPORT_SYMBOL(mac_hwapi_modify_flow_control_pause_pkt_addr);

#if !defined(CONFIG_BCM947622)
void mac_hwapi_set_backpressure_ext(rdpa_emac emacNum,int32_t enable)
{
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM963158)
    S_UNIMAC_TOP_CFG2_REG cfgreg;
    uintptr_t misc_top_address = (uintptr_t)UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2 + UNIMAC_MISC_INSTANCE_OFFSET(emacNum);

    if (!(enabled_emac & (1 << emacNum)))
        return;

    READ_32(misc_top_address,cfgreg);
    cfgreg.backpressure_enable_ext = (enable ? 1 : 0);
    WRITE_32(misc_top_address,cfgreg);
#endif
}
EXPORT_SYMBOL(mac_hwapi_set_backpressure_ext);
#endif //!47622

void mac_hwapi_set_eee(rdpa_emac emacNum, int32_t enable)
{
    uint32_t eee_ref_count = 0;
    uint32_t eee_wake_timer = 0;
    uint32_t eee_lpi_timer = 0;

    S_UNIMAC_EEE_CTRL_REG eee_ctrl;
    S_HWAPI_MAC_STATUS  mac_mode;

    /* Determine EEE timers only when EEE is enabled */
    if (enable)
    {
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM947622) || defined(CONFIG_BCM96878)
        eee_ref_count = 0xfa; /* 250 Mhz */;
#else
        get_rdp_freq(&eee_ref_count);
        eee_ref_count /= 2;
#endif

        /* Read actual mac speed */
        UNIMAC_READ32_REG(emacNum, MODE, mac_mode);

        switch (mac_mode.mac_speed) {
        case rdpa_emac_rate_100m: /* 100Mbps */
            {
                eee_lpi_timer = 0x3c; /* 60 uS */
                eee_wake_timer = 0x1e; /* 30 uS */
                break;
            }
        case rdpa_emac_rate_1g: /* 1000Mbps */
            {
                eee_lpi_timer = 0x22; /* 34 uS */
                eee_wake_timer = 0x11; /* 17 uS */
                break;
            }
        case rdpa_emac_rate_2_5g: /* 2500Mbps */
            {
                eee_lpi_timer = 0x3c; /* 60 uS */
                eee_wake_timer = 0x1e; /* 30 uS */
                break;
            }
        default:
            break;
        }
    }

    UNIMAC_READ32_REG(emacNum, EEE_CTRL, eee_ctrl);
    eee_ctrl.eee_en = enable ? 1 : 0;
    UNIMAC_WRITE32_REG(emacNum, EEE_CTRL, eee_ctrl);
    UNIMAC_WRITE32_REG(emacNum, EEE_REF_COUNT, eee_ref_count);

#if defined(CONFIG_BCM96856) || defined(CONFIG_BCM947622)
    if (mac_mode.mac_speed == rdpa_emac_rate_100m)
    {
        UNIMAC_WRITE32_REG(emacNum, EEE_MII_LPI_TIMER, eee_lpi_timer);
        UNIMAC_WRITE32_REG(emacNum, EEE_MII_WAKE_TIMER, eee_wake_timer);
    }
    else
    {
        UNIMAC_WRITE32_REG(emacNum, EEE_GMII_LPI_TIMER, eee_lpi_timer);
        UNIMAC_WRITE32_REG(emacNum, EEE_GMII_WAKE_TIMER, eee_wake_timer);
    }
#else
    UNIMAC_WRITE32_REG(emacNum, EEE_LPI_TIMER, eee_lpi_timer);
    UNIMAC_WRITE32_REG(emacNum, EEE_WAKE_TIMER, eee_wake_timer);
#endif
}
EXPORT_SYMBOL(mac_hwapi_set_eee);
