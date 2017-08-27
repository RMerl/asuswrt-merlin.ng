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
/* This file contains the implementation of the HwApiMac for 6838 unimac      */
/*                                                                            */
/******************************************************************************/



/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/
#include "rdp_drv_unimac.h"



/******************************************************************************/
/*                                                                            */
/* Types and values definitions                                               */
/*                                                                            */
/******************************************************************************/
#define UNIMAC_SWRESET_ON           1
#define UNIMAC_SWRESET_OFF          0
#define UNIMAC_DEFAULT_IPG          12
#define UNIMAC_DEFAULT_PAUSE_QUANTA 0xffff
#define UNIMAC_DEFAULT_PAUSE_TIMER  0x1ffff
#define UNIMAC_MAX_FRAME_LENGHT 9000

#define UNIMAC_NUMBER_OF_SPEEDS     ( UNIMAC_SPEED_2500 + 1 )
#define UNIMAC_NUMBER_OF_DUPLEXES   ( UNIMAC_DUPLEX_FULL + 1 )

static uint32_t ipgSpeedTable[UNIMAC_NUMBER_OF_SPEEDS][UNIMAC_NUMBER_OF_DUPLEXES] =
{
        {0x000a,0x000a},
        {0x000a,0x000a},
        {0x0005,0x0005},
        {0x0005,0x0005}
};
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
/*   get the configuratin of a mac port ,note that current status of emac     */
/*   might be different than configuration when working in autoneg            */
/*  to get the current status use mac_hwapi_get_mac_status API                */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    emac_cfg -  structure holds the current configuration of the mac  port  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_configuration(rdpa_emac emacNum,rdpa_emac_cfg_t *emac_cfg)
{
    S_UNIMAC_CMD_REG  mCfgReg;

    UNIMAC_READ32_REG(emacNum,CMD,mCfgReg);
    emac_cfg->allow_too_long        =   0;//no such
    emac_cfg->back2back_gap         =   0;//no such
    emac_cfg->check_length          =   !mCfgReg.no_lgth_check;
    emac_cfg->full_duplex           =   !mCfgReg.hd_ena;
    emac_cfg->generate_crc          =   0;
    emac_cfg->loopback              =   mCfgReg.rmt_loop_ena;
    emac_cfg->non_back2back_gap     =   0;//no such
    emac_cfg->pad_short             =   0;//no such
    emac_cfg->rate                  =   mCfgReg.eth_speed;
    emac_cfg->rx_flow_control       =   !mCfgReg.rx_pause_ignore;
    emac_cfg->tx_flow_control       =   !mCfgReg.tx_pause_ignore;

    UNIMAC_READ_FIELD(emacNum,HD_BKP_CNTL,ipg_config_rx,emac_cfg->min_interframe_gap);
    UNIMAC_READ_FIELD(emacNum,TX_IPG_LEN,tx_ipg_len,emac_cfg->preamble_length);
}
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
/*   emacNum - emac Port index                                                */
/*   emac_cfg -  structure holds the current configuration of the mac  port   */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_configuration(rdpa_emac emacNum,rdpa_emac_cfg_t *emac_cfg)
{
    S_UNIMAC_CMD_REG  mCfgReg;

    /*first put the mac in reset to enable config change*/
    UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,1);


    UNIMAC_READ32_REG(emacNum,CMD,mCfgReg);

    /*now mac configuration can be changed */
    if(!mCfgReg.ena_ext_config) /* Change only if auto_config is not set */
    {
          mCfgReg.eth_speed       = emac_cfg->rate;
          mCfgReg.hd_ena          = !emac_cfg->full_duplex;
          mCfgReg.rx_pause_ignore     = !emac_cfg->rx_flow_control;
          mCfgReg.tx_pause_ignore     = 0;//!emac_cfg->tx_flow_control;
          /*set the interframe gap*/
          UNIMAC_WRITE_FIELD(emacNum,HD_BKP_CNTL,ipg_config_rx,ipgSpeedTable[emac_cfg->rate][(unsigned)emac_cfg->full_duplex]);
    }
    else
    {
      /*set the interframe gap*/
          UNIMAC_WRITE_FIELD(emacNum,HD_BKP_CNTL,ipg_config_rx,emac_cfg->min_interframe_gap);
    }

    mCfgReg.no_lgth_check         = !emac_cfg->check_length;
    mCfgReg.pad_en                = emac_cfg->pad_short;
    mCfgReg.rmt_loop_ena          = emac_cfg->loopback;


    UNIMAC_WRITE32_REG(emacNum,CMD,mCfgReg);

    /*release the sw_reset bit*/
    UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,0);

    /* set the preamble length */
    UNIMAC_WRITE_FIELD(emacNum,TX_IPG_LEN,tx_ipg_len,emac_cfg->preamble_length);




}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_duplex                                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get port duplex                                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  get the dulplex of a port                                                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    full_duples  :  1 = full dulplex,0 = half_duplex                        */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_duplex(rdpa_emac emacNum,int32_t *full_duplex)
{
    /*read field and reverse polarity*/
    int32_t fieldData;

    UNIMAC_READ_FIELD(emacNum,CMD,hd_ena,fieldData);
    *full_duplex = !fieldData;
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_duplex                                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port duplex                                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  set the dulplex of a port                                                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    full_duples  :  1 = full dulplex,0 = half_duplex                        */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_duplex(rdpa_emac emacNum,int32_t full_duplex)
{
    S_UNIMAC_CMD_REG  mCfgReg;
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





/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_speed                                                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get port speed rate                                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  get the speed of a port                                                   */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*      rate -   enum of the speed                                            */
/******************************************************************************/
void mac_hwapi_get_speed(rdpa_emac emacNum,rdpa_emac_rate *rate)
{
    /*read the speed field*/
    UNIMAC_READ_FIELD(emacNum,CMD,eth_speed,*rate);
}





/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_speed                                                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port speed rate                                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  set the speed of a port                                                   */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*      rate -   enum of the speed                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/******************************************************************************/

void mac_hwapi_set_speed(rdpa_emac emacNum,rdpa_emac_rate rate)
{
    S_UNIMAC_CMD_REG  mCfgReg;
    UNIMAC_READ32_REG(emacNum,CMD,mCfgReg);
    if (!mCfgReg.ena_ext_config) /* Change only if auto_config is not set */
    {
        UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_ON);

        UNIMAC_WRITE_FIELD(emacNum,CMD,eth_speed,rate);


        UNIMAC_WRITE_FIELD(emacNum,HD_BKP_CNTL,ipg_config_rx,ipgSpeedTable[rate][0]);


        UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_OFF);
    }
}




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_rxtx_enable                                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set tx and rx enable                                        */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  get tx and rx enable                                                      */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*       rxtxEnable  - boolean enable                                         */
/******************************************************************************/
void mac_hwapi_get_rxtx_enable(rdpa_emac emacNum,int32_t *rxEnable,int32_t *txEnable)
{
    UNIMAC_READ_FIELD(emacNum,CMD,tx_ena,*txEnable);
    UNIMAC_READ_FIELD(emacNum,CMD,rx_ena,*rxEnable);

}






/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_rxtx_enable                                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set tx and rx enable                                        */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  set tx and rx enable                                                      */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*    rxtxEnable  - boolean enable                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_rxtx_enable(rdpa_emac emacNum,int32_t rxEnable,int32_t txEnable)
{
    UNIMAC_WRITE_FIELD(emacNum,CMD,tx_ena,txEnable);
    UNIMAC_WRITE_FIELD(emacNum,CMD,rx_ena,rxEnable);
}





/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_sw_reset                                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get port software reset                                     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  get the sw reset bit of emac port                                         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    swReset  :  1 = reset,0 = not reset                                     */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_sw_reset(rdpa_emac emacNum,int32_t *swReset)
{
    UNIMAC_READ_FIELD(emacNum,CMD,sw_reset,*swReset);
}





/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_sw_reset                                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port software reset                                     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  set the sw reset bit of emac port                                         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*   swReset  :  1 = reset,0 = not reset                                      */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_sw_reset(rdpa_emac emacNum,int32_t swReset)
{
    UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,swReset);
}





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
    UNIMAC_READ_FIELD(emacNum,TX_IPG_LEN,tx_min_pkt_size,*min_pkt_size);
}




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
    UNIMAC_WRITE_FIELD(emacNum,TX_IPG_LEN,tx_min_pkt_size,min_pkt_size);
}




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_tx_max_frame_len                                           */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get port TX MTU                                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  get the port maximum transmit unit size in bytes                          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    maxTxFrameLen - size of frame in bytes                                  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_tx_max_frame_len(rdpa_emac emacNum,uint32_t *maxTxFrameLen )
{
    S_UNIMAC_TOP_CFG1_REG cfgreg;
    unsigned int address = UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1 + ( UNIMAC_MISC_EMAC_OFFSET * emacNum );

    READ_32(address,cfgreg);

    *maxTxFrameLen = cfgreg.max_pkt_size;
}





/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_tx_max_frame_len                                           */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port TX MTU                                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  set the port maximum transmit unit size in bytes                          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*    maxTxFrameLen - size of frame in bytes                                  */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_tx_max_frame_len(rdpa_emac emacNum,uint32_t maxTxFrameLen )
{

    S_UNIMAC_TOP_CFG1_REG cfgreg;
    unsigned int address = UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1 + ( UNIMAC_MISC_EMAC_OFFSET * emacNum );

    /*put emac in sw reset state*/
    UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_ON );

    READ_32(address,cfgreg);
    cfgreg.max_pkt_size = maxTxFrameLen;
    WRITE_32(address,cfgreg);

    UNIMAC_WRITE_FIELD(emacNum,FRM_LEN,frame_length,UNIMAC_MAX_FRAME_LENGHT);

    UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_OFF );
}





/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_rx_max_frame_len                                           */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get port RX MTU                                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  get the port maximum receive unit size in bytes                           */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*   maxRxFrameLen - size of current MRU                                      */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_rx_max_frame_len(rdpa_emac emacNum,uint32_t *maxRxFrameLen )
{
    UNIMAC_READ_FIELD(emacNum,RX_MAX_PKT_SIZE,max_pkt_size,*maxRxFrameLen );
}





/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_rx_max_frame_len                                           */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port RX MTU                                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  set the port maximum receive unit size in bytes                           */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*    maxRxFrameLen - size of current MRU                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_rx_max_frame_len(rdpa_emac emacNum,uint32_t maxRxFrameLen )
{
    /*put emac in sw reset state*/
    UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_ON );

    UNIMAC_WRITE_FIELD(emacNum,RX_MAX_PKT_SIZE,max_pkt_size,maxRxFrameLen);

    /*put emac out sw reset state*/
    UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_OFF );
}





/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_tx_igp_len                                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port inter frame gap                                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  set the inter frame gap size in bytes                                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*   txIpgLen - length in bytes                                               */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_tx_igp_len(rdpa_emac emacNum,uint32_t txIpgLen )
{
    /*we assume that txIpgLen called with bits time resolution*/
    uint32_t    newIpg;

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




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_tx_igp_len                                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get port inter frame gap                                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  get the inter frame gap size in bytes                                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    txIpgLen - length in bytes                                              */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_tx_igp_len(rdpa_emac emacNum,uint32_t *txIpgLen )
{
    UNIMAC_READ_FIELD(emacNum,TX_IPG_LEN,tx_ipg_len,*txIpgLen );

}





/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_mac_status                                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port status                                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  set the status of mac                                                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    macStatus  :                                                            */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_mac_status(rdpa_emac emacNum,S_HWAPI_MAC_STATUS *macStatus)
{

}






/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_flow_control                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get port flow control                                       */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  get the flow control of a port                                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*   flowControl - structure with parameters of tx and rx flow control        */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_flow_control(rdpa_emac emacNum,S_MAC_HWAPI_FLOW_CTRL *flowControl)
{
    int32_t rxFlow,txFlow;
    UNIMAC_READ_FIELD(emacNum,CMD,rx_pause_ignore,rxFlow);
    UNIMAC_READ_FIELD(emacNum,CMD,tx_pause_ignore,txFlow);
    flowControl->rxFlowEnable   = !rxFlow;
    flowControl->txFlowEnable   = !txFlow;
}

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_pause_params                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port flow control                                       */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  set the flow control of a port                                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*   flowControl - structure with parameters of tx and rx flow control        */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_pause_params(rdpa_emac emacNum,int32_t pauseCtrlEnable,uint32_t pauseTimer,uint32_t pauseQuanta)
{
    UNIMAC_WRITE_FIELD(emacNum,PAUSE_CNTRL,pause_control_en,pauseCtrlEnable );
    UNIMAC_WRITE_FIELD(emacNum,PAUSE_CNTRL,pause_timer,pauseTimer );
    UNIMAC_WRITE_FIELD(emacNum,PAUSE_QUNAT,pause_quant,pauseQuanta );
}



/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_flow_control                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port flow control                                       */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  set the flow control of a port                                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*   flowControl - structure with parameters of tx and rx flow control        */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_flow_control(rdpa_emac emacNum,S_MAC_HWAPI_FLOW_CTRL *flowControl)
{
    UNIMAC_WRITE_FIELD(emacNum,CMD,rx_pause_ignore, !flowControl->rxFlowEnable);
    if(emacNum==rdpa_emac4)
    {
        UNIMAC_WRITE_FIELD(emacNum,CMD,tx_pause_ignore, 0);
    }
    else
    {
        UNIMAC_WRITE_FIELD(emacNum,CMD,tx_pause_ignore, !flowControl->txFlowEnable);
    }
}





/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_rx_counters                                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver -   get the rx counters of port                               */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  get the rx counters of port                                               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    rxCounters  :  structure filled with counters                           */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_rx_counters(rdpa_emac emacNum,rdpa_emac_rx_stat_t *rxCounters)
{
    uint32_t        tempVal;


    UNIMAC_READ32_MIB(emacNum,GRALN,rxCounters->alignment_error);
    UNIMAC_READ32_MIB(emacNum,GRBCA,rxCounters->broadcast_packet);
    UNIMAC_READ32_MIB(emacNum,GRUC, rxCounters->unicast_packet);
    UNIMAC_READ32_MIB(emacNum,GRBYT,rxCounters->byte);
    UNIMAC_READ32_MIB(emacNum,GRFCR,rxCounters->carrier_sense_error);
    UNIMAC_READ32_MIB(emacNum,GRCDE,rxCounters->code_error);
    UNIMAC_READ32_MIB(emacNum,GRXCF,rxCounters->control_frame);
    UNIMAC_READ32_MIB(emacNum,GRFCS,rxCounters->fcs_error);
    rxCounters->fragments               =   0;//?????
    UNIMAC_READ32_MIB(emacNum,GR255,rxCounters->frame_128_255);
    UNIMAC_READ32_MIB(emacNum,GRMGV,tempVal);
    rxCounters->frame_1519_mtu          =   tempVal;
    UNIMAC_READ32_MIB(emacNum,GR2047,tempVal);
    rxCounters->frame_1519_mtu          +=  tempVal;
    UNIMAC_READ32_MIB(emacNum,GR4095,tempVal);
    rxCounters->frame_1519_mtu          +=  tempVal;
    UNIMAC_READ32_MIB(emacNum,GR9216,tempVal);
    rxCounters->frame_1519_mtu          +=  tempVal;

    UNIMAC_READ32_MIB(emacNum,GR511,rxCounters->frame_256_511);

    UNIMAC_READ32_MIB(emacNum,GR1023,rxCounters->frame_512_1023);

    UNIMAC_READ32_MIB(emacNum,GR1518,rxCounters->frame_1024_1518);

    UNIMAC_READ32_MIB(emacNum,GR64,rxCounters->frame_64);

    UNIMAC_READ32_MIB(emacNum,GR127,rxCounters->frame_65_127);

    UNIMAC_READ32_MIB(emacNum,GRFLR,rxCounters->frame_length_error);

    UNIMAC_READ32_MIB(emacNum,GRJBR,rxCounters->jabber);

    UNIMAC_READ32_MIB(emacNum,GRMCA,rxCounters->multicast_packet);

    rxCounters->overflow                =   0;//no matched counter
    UNIMAC_READ32_MIB(emacNum,GROVR,rxCounters->oversize_packet);
    rxCounters->undersize_packet        =   0;//no matched counter
    UNIMAC_READ32_MIB(emacNum,GRPOK,rxCounters->packet);

    UNIMAC_READ32_MIB(emacNum,GRXPF,rxCounters->pause_control_frame);

    UNIMAC_READ32_MIB(emacNum,GRXUO,rxCounters->unknown_opcode);


    /*reset RX counters on hw */
    UNIMAC_WRITE_MIB_FIELD(emacNum,MIB_CNTRL,rx_cnt_st,1);

    UNIMAC_WRITE_MIB_FIELD(emacNum,MIB_CNTRL,rx_cnt_st,0);
}





/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_tx_counters                                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver -   get the tx counters of port                               */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  get the tx counters of port                                               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    txCounters  :  structure filled with counters                           */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_tx_counters(rdpa_emac emacNum,rdpa_emac_tx_stat_t *txCounters)
{

  /*fill the rdpa counters structure*/
    uint32_t        tempVal;


    UNIMAC_READ32_MIB(emacNum,GTBYT,txCounters->byte);

    UNIMAC_READ32_MIB(emacNum,GTBCA,txCounters->broadcast_packet);

    UNIMAC_READ32_MIB(emacNum,GTMCA, txCounters->multicast_packet);

    UNIMAC_READ32_MIB(emacNum,GTUC, txCounters->unicast_packet);

    UNIMAC_READ32_MIB(emacNum,GTXCF,txCounters->control_frame);

    UNIMAC_READ32_MIB(emacNum,GTDRF,txCounters->deferral_packet);


    UNIMAC_READ32_MIB(emacNum,GTNCL,tempVal);
    txCounters->error                       = tempVal;
    UNIMAC_READ32_MIB(emacNum,GTOVR,tempVal);
    txCounters->error                       += tempVal;
    UNIMAC_READ32_MIB(emacNum,GTFCS,tempVal);
    txCounters->error                       += tempVal;
    UNIMAC_READ32_MIB(emacNum,GTEDF,txCounters->excessive_deferral_packet);

    UNIMAC_READ32_MIB(emacNum,GTFCS,txCounters->fcs_error   );

    UNIMAC_READ32_MIB(emacNum,GTFRG,txCounters->fragments_frame);

    UNIMAC_READ32_MIB(emacNum,TR1518,txCounters->frame_1024_1518);

    UNIMAC_READ32_MIB(emacNum,TR255,  txCounters->frame_128_255);

    UNIMAC_READ32_MIB(emacNum,TR2047,  tempVal);
    txCounters->frame_1519_mtu          =   tempVal;
    UNIMAC_READ32_MIB(emacNum,TR4095,  tempVal);
    txCounters->frame_1519_mtu          +=   tempVal;
    UNIMAC_READ32_MIB(emacNum,TR9216,  tempVal);
    txCounters->frame_1519_mtu          +=   tempVal;
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

    txCounters->underrun                    =   0;
    txCounters->undersize_frame             =   0;

    UNIMAC_READ32_MIB(emacNum,GTFRG, txCounters->fragments_frame);


    /*reset TX counters on hw */
    UNIMAC_WRITE_MIB_FIELD(emacNum,MIB_CNTRL,tx_cnt_rst,1);

    UNIMAC_WRITE_MIB_FIELD(emacNum,MIB_CNTRL,tx_cnt_rst,0);
}

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_init_emac                                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver -   init the emac to well known state                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  initialized the emac port                                                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_init_emac(rdpa_emac emacNum)
{
    /*configure the emac and put it in automatic speed mode*/
    S_UNIMAC_CMD_REG    mCfgReg;

    /*before calling this function make sure you pull out of reset the emac */

    /*put the emac in sw_reset state*/
    //UNIMAC_WRITE_FIELD(emacNum,CMD,sw_reset,UNIMAC_SWRESET_ON);

    UNIMAC_READ32_REG(emacNum,CMD,mCfgReg);
    /* Do the initialization */
    mCfgReg.tx_ena          =   0;
    mCfgReg.rx_ena          =   0;
    /* for 63138 - even though the EMAC_1 is connected to SF2 @ 2Gbps, we still set the link to 1G.
     * Actual speed of the link is derived from SF2 based on the speed set for IMP port#8.  */
    mCfgReg.eth_speed       =   UNIMAC_SPEED_1000;
    mCfgReg.promis_en       =   1;
    mCfgReg.pad_en          =   0;
    mCfgReg.pause_fwd       =   1;
    mCfgReg.crc_fwd         =   1;
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    mCfgReg.ena_ext_config  =   0;
#else
    mCfgReg.ena_ext_config  =   (emacNum != rdpa_emac4) && (emacNum != rdpa_emac5) ? 1 : 0;
#endif
    mCfgReg.rx_pause_ignore =   0;
    mCfgReg.tx_pause_ignore =   0;
    mCfgReg.tx_addr_ins     =   0;
    mCfgReg.lcl_loop_ena    =   0;
    mCfgReg.cntl_frm_ena    =   1;
    mCfgReg.no_lgth_check   =   1;
    mCfgReg.rmt_loop_ena    =   0;
    mCfgReg.rx_err_disc     =   0;
    mCfgReg.prbl_ena        =   0;
    mCfgReg.cntl_frm_ena    =   1;

    /*write the configuration */
    UNIMAC_WRITE32_REG(emacNum,CMD,mCfgReg);

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
    /*emac is ready to go!*/
}

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_loopback                                                  */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver -   init the emac to well known state                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  initialized the emac port                                                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_loopback(rdpa_emac emacNum,MAC_LPBK *loopback)
{
    S_UNIMAC_CMD_REG    mCmdReg;

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

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_loopback                                                  */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver -   set loopback type of the mac                              */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  initialized the emac port                                                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_loopback(rdpa_emac emacNum,MAC_LPBK loopback)
{
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

void mac_hwapi_set_unimac_cfg(rdpa_emac emacNum)
{
    S_UNIMAC_CFG_REG cfgreg;
    unsigned int address = UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG + ( UNIMAC_MISC_EMAC_OFFSET * emacNum );

    READ_32(address,cfgreg);
    cfgreg.gmii_direct = 1;
    WRITE_32(address,cfgreg);
}

void mac_hwapi_modify_flow_control_pause_pkt_addr ( rdpa_emac emacNum,bdmf_mac_t mac)
{
    uint32_t value = *(uint32_t*)(&(mac.b[4])) >> 16 ;
    uint32_t misc_top_address;
    uint32_t mac_address = (uint32_t)(UNIMAC_CONFIGURATION_UMAC_0_RDP_MAC0 + ( UNIMAC_CONF_EMAC_OFFSET * emacNum ));

    WRITE_32(mac_address, *(uint32_t*)mac.b);

    mac_address = (uint32_t)(UNIMAC_CONFIGURATION_UMAC_0_RDP_MAC1 + ( UNIMAC_CONF_EMAC_OFFSET * emacNum ));
    WRITE_32(mac_address, value);

    misc_top_address = (uint32_t)(UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2 + ( UNIMAC_MISC_EMAC_OFFSET * emacNum ));
    READ_32(misc_top_address, value);

    value |= UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_INT;
    WRITE_32(misc_top_address, value);
}


void mac_hwapi_set_backpressure_ext(rdpa_emac emacNum,int32_t enable)
{
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    S_UNIMAC_TOP_CFG2_REG cfgreg;
    uint32_t misc_top_address = (uint32_t)(UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2 + ( UNIMAC_MISC_EMAC_OFFSET * emacNum ));

    READ_32(misc_top_address,cfgreg);
    cfgreg.backpressure_enable_ext = (enable ? 1 : 0);
    WRITE_32(misc_top_address,cfgreg);
#endif
}

#if defined(__KERNEL__) && defined(WL4908)
EXPORT_SYMBOL(mac_hwapi_set_configuration);
EXPORT_SYMBOL(mac_hwapi_get_configuration);
EXPORT_SYMBOL(mac_hwapi_get_rx_counters);
EXPORT_SYMBOL(mac_hwapi_get_tx_counters);
EXPORT_SYMBOL(mac_hwapi_set_rx_max_frame_len);
EXPORT_SYMBOL(mac_hwapi_get_rxtx_enable);
EXPORT_SYMBOL(mac_hwapi_set_rxtx_enable);
#endif

