#include "bp_defs.h"
#include "boardparms.h"
#include "bcmSpiRes.h"
extern BpCmdElem moca6802InitSeq[];

//E8C reference design , 2L PCB, 4FE, 1xUSB, 1xWiFi, GPON, SIM CARD, NAND
static bp_elem_t g_bcm968380fhgu[] = {
  {bp_cpBoardId,               .u.cp = "968380FHGU"},  
  {bp_ulOpticalWan,            .u.ul = BP_OPTICAL_WAN_GPON},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_72_AL},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_47_AL},
  {bp_usExtIntrWifiOnOff ,     .u.us = BP_EXT_INTR_2},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_71_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_4_AL},
  {bp_usGpioVoip1Led,          .u.us = BP_LED_2_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_10_AL},
  {bp_usGpioLedGpon,           .u.us = BP_LED_5_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_15_AL},
  {bp_usGpioLedEpon,           .u.us = BP_LED_5_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_15_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_LED_6_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_17_AL},
  {bp_usGpioLedOpticalLinkFail,.u.us = BP_LED_0_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_33_AL},
  {bp_usGpioLedUSB,            .u.us = BP_GPIO_53_AL},
  {bp_ucPhyType0,              .u.uc = BP_ENET_INTERNAL_PHY},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = 0x01 | MAC_IF_MII},
  {bp_ulPhyId1,                .u.ul = 0x02 | MAC_IF_MII},
  {bp_ulPhyId2,                .u.ul = 0x03 | MAC_IF_MII},
  {bp_ulPhyId3,                .u.ul = 0x04 | MAC_IF_MII},
  {bp_ulSimInterfaces,         .u.us = BP_SIMCARD_GROUPA},
  {bp_ulSlicInterfaces,        .u.us = BP_SLIC_GROUPD},
  {bp_usGpioPonTxEn,           .u.us = BP_GPIO_13_AH},
  {bp_usGpioPonRxEn,           .u.us = BP_GPIO_13_AH},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0}, 
  {bp_usRogueOnuEn,            .u.us = 1},
  {bp_usGpioWanSignalDetected, .u.us = BP_GPIO_11_AH},
  {bp_last}
};

//BCM968380FHGU_SI board.
static bp_elem_t g_bcm968380fhgu_si[] = {
  {bp_cpBoardId,               .u.cp = "968380FHGU_SI"},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm968380fhgu},
  {bp_last}
};

//E8C reference design, 2L PCB,  4GE, 1xUSB, 1xWiFi, GPON, SIM CARD, NAND
static bp_elem_t g_bcm968380fggu[] = {
  {bp_cpBoardId,               .u.cp = "968380FGGU"},
  {bp_ucPhyType0,              .u.uc = BP_ENET_INTERNAL_PHY},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = 0x01 | MAC_IF_GMII},
  {bp_ulPhyId1,                .u.ul = 0x02 | MAC_IF_GMII},
  {bp_ulPhyId2,                .u.ul = 0x03 | MAC_IF_GMII},
  {bp_ulPhyId3,                .u.ul = 0x04 | MAC_IF_GMII},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm968380fhgu},
  {bp_last}
};

static bp_elem_t g_bcm968380fggu_tri[] = {
  {bp_cpBoardId,               .u.cp = "968380FGGU_TRI"},
  {bp_ucPhyType0,              .u.uc = BP_ENET_INTERNAL_PHY},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x2f},
  {bp_ulPhyId0,                .u.ul = 0x01 | MAC_IF_GMII},
  {bp_ulPhyId1,                .u.ul = 0x02 | MAC_IF_GMII},
  {bp_ulPhyId2,                .u.ul = 0x03 | MAC_IF_GMII},
  {bp_ulPhyId3,                .u.ul = 0x04 | MAC_IF_GMII},
  {bp_ulPhyId5,                .u.ul = 0x02 | BCM_WAN_PORT | MAC_IF_SERDES}, /* WAN interface */
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_DETECT | PORT_FLAG_WAN_ONLY},
  {bp_ucPhyDevName,            .u.cp = "eth5"},
  {bp_usGpioPonTxEn,           .u.us = BP_GPIO_13_AL},
  {bp_usGpioPonRxEn,           .u.us = BP_GPIO_13_AL},
  {bp_usAePolarity,            .u.us = 1},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm968380fhgu},
  {bp_last}
};

//E8C reference design, 4L PCB, 4GbE, 1xUSB, 1xWiFi, GPON, SIM CARD, NAND
static bp_elem_t g_bcm968380ffhg[] = {
  {bp_cpBoardId,               .u.cp = "968380FFHG"},
  {bp_ulOpticalWan,            .u.ul = BP_OPTICAL_WAN_GPON},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm968380fggu},
  {bp_last}
};

//E8C Reference design - 4L PCB, 4xGbE, 2xUSB, 2xWiFi, GPON, SIM CARD, 2xFXS
static bp_elem_t g_bcm968380gerg[] = {
  {bp_cpBoardId,               .u.cp = "968380GERG"},
  {bp_ulOpticalWan,            .u.ul = BP_OPTICAL_WAN_GPON},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_47_AL},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_39_AL},
  {bp_usExtIntrWifiOnOff,      .u.us = BP_EXT_INTR_2},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_48_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_4_AL},
  {bp_usGpioVoip1Led,          .u.us = BP_LED_2_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_10_AL},
  {bp_usGpioLedGpon,           .u.us = BP_LED_5_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_15_AL},
  {bp_usGpioLedEpon,           .u.us = BP_LED_5_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_15_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_LED_6_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_17_AL},
  {bp_usGpioLedOpticalLinkFail,.u.us = BP_LED_0_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_33_AL},
  {bp_usGpioLedUSB,            .u.us = BP_LED_3_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_11_AL},
  {bp_ucPhyType0,              .u.uc = BP_ENET_INTERNAL_PHY},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = 0x01 | MAC_IF_GMII},
  {bp_ulPhyId1,                .u.ul = 0x02 | MAC_IF_GMII},
  {bp_ulPhyId2,                .u.ul = 0x03 | MAC_IF_GMII},
  {bp_ulPhyId3,                .u.ul = 0x04 | MAC_IF_GMII},
  {bp_ulSimInterfaces,         .u.us = BP_SIMCARD_GROUPA},
  {bp_ulSlicInterfaces,        .u.us = BP_SLIC_GROUPD},
  {bp_usGpioPonTxEn,           .u.us = BP_GPIO_13_AH},
  {bp_usGpioPonRxEn,           .u.us = BP_GPIO_13_AH},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usRogueOnuEn,            .u.us = 1},
  {bp_usGpioWanSignalDetected, .u.us = BP_GPIO_53_AH},
  {bp_last}
};

//BCM968380MGEG board.
static bp_elem_t g_bcm968380mgeg[] = {
  {bp_cpBoardId,               .u.cp = "968380MGEG"},
  {bp_usGpioLedGpon,           .u.us = BP_GPIO_NONE},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm968380gerg},
  {bp_last}
};

//BCM968380GERG_SI board.
static bp_elem_t g_bcm968380gerg_si[] = {
  {bp_cpBoardId,               .u.cp = "968380GERG_SI"},
  {bp_usSpiSlaveSelectNum,     .u.us = 2}, /* define the SPI select for voice */
  {bp_usSpiSlaveSelectGpioNum, .u.us = 4},  
  {bp_elemTemplate,            .u.bp_elemp = g_bcm968380gerg},
  {bp_last}
};

//BCM968380F SV Board, big 6838SV board , GPON
static bp_elem_t g_bcm968380fsv_g[] = {
  {bp_cpBoardId,               .u.cp = "968380FSV_G"},
//for Active Ethernet use
  {bp_ulOpticalWan,            .u.ul = BP_OPTICAL_WAN_AE},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_INTERNAL_PHY},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
//for active ethernet use
  {bp_ulPortMap,               .u.ul = 0x3f},
  {bp_ulPhyId0,                .u.ul = 0x01 | MAC_IF_GMII},
  {bp_ulPhyId1,                .u.ul = 0x02 | MAC_IF_GMII},
  {bp_ulPhyId2,                .u.ul = 0x03 | MAC_IF_GMII},
  {bp_ulPhyId3,                .u.ul = 0x04 | MAC_IF_GMII},
  {bp_ulPhyId4,                .u.ul = 0x18 | MAC_IF_GMII | PHY_EXTERNAL | PHY_INTEGRATED_VALID},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_TX_INTERNAL_DELAY},
//for active ethernet use
  {bp_ulPhyId5,                .u.ul = 0x2 | BCM_WAN_PORT | PHYCFG_VALID | MAC_IF_SERDES}, /* WAN interface */
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_DETECT | PORT_FLAG_WAN_ONLY},
  {bp_usGpioPonTxEn,           .u.us = BP_GPIO_1_AH},
  {bp_usGpioPonRxEn,           .u.us = BP_GPIO_0_AH},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usTsync1025mhz,          .u.us = BP_PIN_TSYNC_1025MHZ_11},
  {bp_usTsync8khz,             .u.us = BP_PIN_TSYNC_8KHZ_4},
  {bp_usTsync1pps,             .u.us = BP_PIN_TSYNC_1PPS_6},
  {bp_usGpioTsyncPonUnstable,  .u.us = BP_GPIO_7_AH},
  {bp_usGpioUart2Sdin,         .u.us = BP_GPIO_14_AH}, // uncomment to enable second UART
  {bp_usGpioUart2Sdout,        .u.us = BP_GPIO_15_AH}, // uncomment to enable second UART       
//{bp_usSerialLEDMuxSel,       .u.us = BP_SERIAL_LED_MUX_GROUPA}, an example of us eof shift register to output LEDs
  {bp_usSpiSlaveSelectNum,     .u.us = 4}, /* define the SPI select for voice */
  {bp_usSpiSlaveSelectGpioNum, .u.us = 9},
  {bp_usSpiSlaveSelectNum,     .u.us = 6},
  {bp_usSpiSlaveSelectGpioNum, .u.us = 12},
  {bp_usSpiSlaveSelectNum,     .u.us = 7},
  {bp_usSpiSlaveSelectGpioNum, .u.us = 67},
  {bp_last}
};

//TBD 
static bp_elem_t g_bcm968380sv_g[] = {
  {bp_cpBoardId,               .u.cp = "968380SV_G"},
  {bp_ulOpticalWan,            .u.ul = BP_OPTICAL_WAN_GPON},
  {bp_usSpiSlaveSelectNum,     .u.us = 4}, /* define the SPI select for voice */
  {bp_usSpiSlaveSelectGpioNum, .u.us = 9},
  {bp_usSpiSlaveSelectNum,     .u.us = 6},
  {bp_usSpiSlaveSelectGpioNum, .u.us = 12},
  {bp_usSpiSlaveSelectNum,     .u.us = 7},
  {bp_usSpiSlaveSelectGpioNum, .u.us = 67},  
  {bp_elemTemplate,            .u.bp_elemp = g_bcm968380fsv_g}, //?????
  {bp_last}
};

//E8C Reference design  2L PCB, GPON, 2xFE, 1x FXS, SIM CARD, NAND, SPI NOR
static bp_elem_t g_bcm968385sfu[] = {
  {bp_cpBoardId,               .u.cp = "968385SFU"},
  {bp_ulOpticalWan,            .u.ul = BP_OPTICAL_WAN_GPON},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_48_AL},
  {bp_usGpioVoip1Led,          .u.us = BP_LED_2_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_10_AL},
  {bp_usGpioLedGpon,           .u.us = BP_LED_5_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_15_AL},
  {bp_usGpioLedEpon,           .u.us = BP_LED_5_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_15_AL},
  {bp_usGpioLedOpticalLinkFail,.u.us = BP_LED_0_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_33_AL},
  {bp_usGpioLedSim,            .u.us = BP_GPIO_4_AL},
  {bp_usGpioLedSim_ITMS,       .u.us = BP_GPIO_9_AL},
  {bp_ucPhyType0,              .u.uc = BP_ENET_INTERNAL_PHY},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x3},
  {bp_ulPhyId0,                .u.ul = 0x01 | MAC_IF_MII},
  {bp_ulPhyId1,                .u.ul = 0x02 | MAC_IF_MII},
  {bp_ulPhyId2,                .u.ul = 0x03 | MAC_IF_MII},
  {bp_ulPhyId3,                .u.ul = 0x04 | MAC_IF_MII},
  {bp_ulSimInterfaces,         .u.us = BP_SIMCARD_GROUPA},
  {bp_ulSlicInterfaces,        .u.us = BP_SLIC_GROUPD},
  {bp_usGpioPonTxEn,           .u.us = BP_GPIO_13_AH},
  {bp_usGpioPonRxEn,           .u.us = BP_GPIO_13_AH},
  {bp_usRogueOnuEn,            .u.us = 1},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioWanSignalDetected, .u.us = BP_GPIO_11_AH},
  {bp_last}
};

//BCM968385SFU_SI board.
static bp_elem_t g_bcm968385sfu_si[] = {
  {bp_cpBoardId,               .u.cp = "968385SFU_SI"},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm968385sfu},
  {bp_last}
};

//E8C Reference design  2L PCB, GPON, 1x GbE, SIM CARD, NAND, SPI NOR
static bp_elem_t g_bcm968385gsp[] = {
  {bp_cpBoardId,               .u.cp = "968385GSP"},
  {bp_ulOpticalWan,            .u.ul = BP_OPTICAL_WAN_GPON},
  {bp_usGpioVoip1Led,          .u.us = BP_GPIO_NONE},
  {bp_ucDspType0,              .u.uc = BP_VOIP_NO_DSP},
  {bp_ucPhyType0,              .u.uc = BP_ENET_INTERNAL_PHY},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x1},
  {bp_ulPhyId0,                .u.ul = 0x01 | MAC_IF_GMII},
  {bp_ulPhyId1,                .u.ul = 0x02 | MAC_IF_GMII},
  {bp_ulPhyId2,                .u.ul = 0x03 | MAC_IF_GMII},
  {bp_ulPhyId3,                .u.ul = 0x04 | MAC_IF_GMII},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm968385sfu},
  {bp_last}
};

//R&D only
static bp_elem_t g_bcm968385sv_g[] = {
  {bp_cpBoardId,               .u.cp = "968385SV_G"},
  {bp_ulOpticalWan,            .u.ul = BP_OPTICAL_WAN_GPON},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_INTERNAL_PHY},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0xf},
  {bp_ulPhyId0,                .u.ul = 0x01 | MAC_IF_GMII},
  {bp_ulPhyId1,                .u.ul = 0x02 | MAC_IF_GMII},
  {bp_ulPhyId1,                .u.ul = 0x03 | MAC_IF_GMII},
  {bp_ulPhyId1,                .u.ul = 0x04 | MAC_IF_GMII},
//  {bp_ulPhyId2,                .u.ul = 0x00 | PHYCFG_VALID |MAC_IF_GMII | PHY_EXTERNAL},
  {bp_usGpioPonTxEn,           .u.us = BP_GPIO_4_AH},
  {bp_usGpioPonRxEn,           .u.us = BP_GPIO_12_AH},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usSpiSlaveSelectNum,     .u.us = 4}, /* define the SPI select for voice */
  {bp_usSpiSlaveSelectGpioNum, .u.us = 9},
  {bp_usSpiSlaveSelectNum,     .u.us = 6},
  {bp_usSpiSlaveSelectGpioNum, .u.us = 12},  
  {bp_last}
};

static bp_elem_t g_bcm968381sv_g[] = {
  {bp_cpBoardId,               .u.cp = "968381SV_G"},
  {bp_ucDspType0,              .u.uc = BP_VOIP_NO_DSP},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm968385sv_g},
  {bp_last}
};

static bp_elem_t g_bcm968380fttdps[] = {
  {bp_cpBoardId,               .u.cp = "968380FTTDPS"},
  {bp_ulOpticalWan,            .u.ul = BP_OPTICAL_WAN_GPON},
  {bp_ucDspType0,              .u.uc = BP_VOIP_NO_DSP},
  {bp_ucPhyType0,              .u.uc = BP_ENET_INTERNAL_PHY},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x13},
  {bp_ulPhyId0,                .u.ul = 0x01 | MAC_IF_GMII},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_ATTACHED},
  {bp_ulPhyId1,                .u.ul = 0x02 | MAC_IF_GMII},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT},
  {bp_ulPhyId4,                .u.ul = RGMII_DIRECT},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_ATTACHED},
  {bp_ulAttachedIdx,           .u.ul = 0},
/*
  {bp_ulPhyId0,                .u.ul = 0x7},
  {bp_ucPhyDevName,            .u.cp = "testme"},
  {bp_ulPhyId1,                .u.ul = 0x8},
*/
  {bp_ulAttachedIdx,           .u.ul = 4},
  {bp_ulPhyId0,                .u.ul = 0x0},
  {bp_ulPhyId1,                .u.ul = 0x1},
  {bp_ulPhyId2,                .u.ul = 0x2},
  {bp_ulPhyId3,                .u.ul = 0x3},
  {bp_usGpioLedGpon,           .u.us = BP_GPIO_16_AL},
  {bp_usGpioPonTxEn,           .u.us = BP_GPIO_13_AH},
  {bp_usGpioPonRxEn,           .u.us = BP_GPIO_13_AH},
  {bp_usGpioSpromClk,          .u.us = BP_GPIO_68_AH}, /* for FTTDP */
  {bp_usGpioSpromData,         .u.us = BP_GPIO_69_AH}, /* for FTTDP */
  {bp_usGpioAttachedDevReset,  .u.us = BP_GPIO_0_AH}, /* for FTTDP */
  {bp_usRogueOnuEn,            .u.us = 1},
  {bp_usGpioLedOpticalLinkFail,.u.us = BP_LED_0_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_33_AL},
  {bp_usTsync1pps,             .u.us = BP_PIN_TSYNC_1PPS_6},
  {bp_last}
};

static bp_elem_t g_bcm968380dp2[] = {
  {bp_cpBoardId,               .u.cp = "968380DP2"},
  {bp_ulOpticalWan,            .u.ul = BP_OPTICAL_WAN_GPON},
  {bp_ucDspType0,              .u.uc = BP_VOIP_NO_DSP},
  {bp_ucPhyType0,              .u.uc = BP_ENET_INTERNAL_PHY},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x11},
  {bp_ulPhyId0,                .u.ul = 0x01 | MAC_IF_GMII},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT},
  {bp_ulPhyId4,                .u.ul = RGMII_DIRECT},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_ATTACHED},
  {bp_ulAttachedIdx,           .u.ul = 4},
  {bp_ulPhyId0,                .u.ul = 0x0},
  {bp_ulPhyId1,                .u.ul = 0x1},
  {bp_usGpioLedGpon,           .u.us = BP_GPIO_16_AL},
  {bp_usGpioPonTxEn,           .u.us = BP_GPIO_13_AH},
  {bp_usGpioPonRxEn,           .u.us = BP_GPIO_13_AH},
  {bp_usGpioSpromClk,          .u.us = BP_GPIO_68_AH}, /* for FTTDP */
  {bp_usGpioSpromData,         .u.us = BP_GPIO_69_AH}, /* for FTTDP */
  {bp_usGpioAttachedDevReset,  .u.us = BP_GPIO_67_AH}, /* for FTTDP */
  {bp_usRogueOnuEn,            .u.us = 1},
  {bp_usGpioLedOpticalLinkFail,.u.us = BP_LED_0_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_33_AL},
  {bp_usTsync1pps,             .u.us = BP_PIN_TSYNC_1PPS_6},
  {bp_last}
};

static bp_elem_t g_bcm965200f_cpe[] = {
  {bp_cpBoardId,               .u.cp = "965200F_CPE"},
  {bp_cpComment,               .u.cp = "(FCOPE_CPE)"},
  {bp_ucDspType0,              .u.uc = BP_VOIP_NO_DSP},
  {bp_ucPhyType0,              .u.uc = BP_ENET_INTERNAL_PHY},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x17},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_1 | PHY_INTERNAL | PHY_INTEGRATED_VALID | MAC_IF_GMII},
  {bp_ulPhyId1,                .u.ul = BP_PHY_ID_2 | PHY_INTERNAL | PHY_INTEGRATED_VALID | MAC_IF_GMII},
  {bp_ulPhyId2,                .u.ul = BP_PHY_ID_3 | PHY_INTERNAL | PHY_INTEGRATED_VALID | MAC_IF_GMII},
  {bp_ulPhyId4,                .u.ul = RGMII_DIRECT},
  {bp_ucPhyDevName,            .u.cp = "eth4"},
  {bp_usSpiSlaveBusNum,        .u.us = HS_SPI_BUS_NUM},
  {bp_usSpiSlaveMode,          .u.us = SPI_MODE_3},
  {bp_ulSpiSlaveCtrlState,     .u.ul = SPI_CONTROLLER_STATE_GATE_CLK_SSOFF},
  {bp_ulSpiSlaveMaxFreq,       .u.ul = 20000000},
  {bp_usSpiSlaveProtoRev,      .u.us = 2},
  {bp_usSpiSlaveSelectNum,     .u.us = 0},
  {bp_usGpioSpiSlaveReset,     .u.us = BP_GPIO_67_AL},
  {bp_last}
};

static bp_elem_t g_bcm965200f_co[] = {
  {bp_cpBoardId,               .u.cp = "965200F_CO"},
  {bp_cpComment,               .u.cp = "(FCOPE_CO)"},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm965200f_cpe},
  {bp_last}
};

static bp_elem_t g_bcm965200dpf[] = {
  {bp_cpBoardId,               .u.cp = "965200DPF"},
  {bp_cpComment,               .u.cp = "(12L-CO,g.fast)"},
  {bp_ulOpticalWan,            .u.ul = BP_OPTICAL_WAN_GPON},
  {bp_ucDspType0,              .u.uc = BP_VOIP_NO_DSP},
  {bp_ucPhyType0,              .u.uc = BP_ENET_INTERNAL_PHY},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_1 | PHY_INTERNAL | PHY_INTEGRATED_VALID | MAC_IF_GMII},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT},
  {bp_ulPhyId1,                .u.ul = BP_PHY_ID_2 | PHY_INTERNAL | PHY_INTEGRATED_VALID | MAC_IF_GMII},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_ATTACHED},
  {bp_ulPhyId2,                .u.ul = BP_PHY_ID_3 | PHY_INTERNAL | PHY_INTEGRATED_VALID | MAC_IF_GMII},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_ATTACHED},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_4 | PHY_INTERNAL | PHY_INTEGRATED_VALID | MAC_IF_GMII},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_ATTACHED},
  {bp_ulPhyId4,                .u.ul = RGMII_DIRECT},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_ATTACHED},
  //{bp_ucPhyDevName,            .u.cp = "eth4"},
  {bp_ulAttachedIdx,           .u.ul = 1},
  {bp_ulPhyId0,                .u.ul = 6},
  {bp_ulPhyId1,                .u.ul = 7},
  {bp_ulPhyId2,                .u.ul = 8},
  {bp_ulAttachedIdx,           .u.ul = 2},
  {bp_ulPhyId0,                .u.ul = 3},
  {bp_ulPhyId1,                .u.ul = 4},
  {bp_ulPhyId2,                .u.ul = 5},
  {bp_ulAttachedIdx,           .u.ul = 3},
  {bp_ulPhyId0,                .u.ul = 0},
  {bp_ulPhyId1,                .u.ul = 1},
  {bp_ulPhyId2,                .u.ul = 2},
  {bp_ulAttachedIdx,           .u.ul = 4},
  {bp_ulPhyId0,                .u.ul = 9},
  {bp_ulPhyId1,                .u.ul = 10},
  {bp_ulPhyId2,                .u.ul = 11},
  {bp_usSpiSlaveBusNum,        .u.us = HS_SPI_BUS_NUM},
  {bp_usSpiSlaveMode,          .u.us = SPI_MODE_3},
  {bp_ulSpiSlaveCtrlState,     .u.ul = SPI_CONTROLLER_STATE_GATE_CLK_SSOFF},
  {bp_ulSpiSlaveMaxFreq,       .u.ul = 20000000},
  {bp_usSpiSlaveProtoRev,      .u.us = 2},
  {bp_usSpiSlaveSelectNum,     .u.us = 0},
  {bp_usGpioSpiSlaveReset,     .u.us = BP_GPIO_67_AL},
  {bp_usSpiSlaveSelectNum,     .u.us = 1},
  {bp_usGpioSpiSlaveReset,     .u.us = BP_GPIO_68_AL},
  {bp_usSpiSlaveSelectNum,     .u.us = 4},
  {bp_usGpioSpiSlaveReset,     .u.us = BP_GPIO_72_AL},
  {bp_usGpioLedGpon,           .u.us = BP_GPIO_16_AL},
  {bp_usGpioPonTxEn,           .u.us = BP_GPIO_13_AH},
  {bp_usGpioPonRxEn,           .u.us = BP_GPIO_13_AH},
  {bp_usRogueOnuEn,            .u.us = 1},
  {bp_usTsync8khz,             .u.us = BP_PIN_TSYNC_8KHZ_4},
  {bp_usTsync1pps,             .u.us = BP_PIN_TSYNC_1PPS_6},
  {bp_usXdResetGpio,           .u.us = BP_GPIO_67_AL},
  {bp_cpXdResetName,           .u.cp = "DSP0_RST_N"},
  {bp_usXdResetReleaseOnInit,  .u.us = 1},
  {bp_usXdResetGpio,           .u.us = BP_GPIO_68_AL},
  {bp_cpXdResetName,           .u.cp = "DSP1_RST_N"},
  {bp_usXdResetReleaseOnInit,  .u.us = 1},
  {bp_usXdResetGpio,           .u.us = BP_GPIO_72_AL},
  {bp_cpXdResetName,           .u.cp = "DSP2_RST_N"},
  {bp_usXdResetReleaseOnInit,  .u.us = 1},
  {bp_usXdResetGpio,           .u.us = BP_GPIO_69_AL},
  {bp_cpXdResetName,           .u.cp = "AFE_RST_N"},
  {bp_usXdResetReleaseOnInit,  .u.us = 1},
  {bp_usXdResetGpio,           .u.us = BP_GPIO_70_AL},
  {bp_cpXdResetName,           .u.cp = "BCM54240_RST_N"},
  {bp_usXdResetReleaseOnInit,  .u.us = 1},
  {bp_usXdResetGpio,           .u.us = BP_GPIO_71_AL},
  {bp_cpXdResetName,           .u.cp = "BCM54220_RST_N"},
  {bp_usXdResetReleaseOnInit,  .u.us = 1},
  {bp_usXdGpio,                .u.us = BP_GPIO_54_AH},
  {bp_usXdGpioInitValue,       .u.us = 0},
  {bp_cpXdGpioInfo,            .u.cp = "DSP_CLK_SEL"},
  {bp_cpXdGpioInfoValue0,      .u.cp = "DCXO"},
  {bp_cpXdGpioInfoValue1,      .u.cp = "VCXO"},
  {bp_usXdGpio,                .u.us = BP_GPIO_2_AH},
  {bp_usXdGpioInitValue,       .u.us = 1},
  {bp_cpXdGpioInfo,            .u.cp = "DSP0_PWR"},
  {bp_cpXdGpioInfoValue0,      .u.cp = "disable core power"},
  {bp_cpXdGpioInfoValue1,      .u.cp = "enable core power"},
  {bp_usXdGpio,                .u.us = BP_GPIO_3_AH},
  {bp_usXdGpioInitValue,       .u.us = 1},
  {bp_cpXdGpioInfo,            .u.cp = "DSP1_PWR"},
  {bp_cpXdGpioInfoValue0,      .u.cp = "disable core power"},
  {bp_cpXdGpioInfoValue1,      .u.cp = "enable core power"},
  {bp_usXdGpio,                .u.us = BP_GPIO_7_AH},
  {bp_usXdGpioInitValue,       .u.us = 1},
  {bp_cpXdGpioInfo,            .u.cp = "DSP2_PWR"},
  {bp_cpXdGpioInfoValue0,      .u.cp = "disable core power"},
  {bp_cpXdGpioInfoValue1,      .u.cp = "enable core power"},
  {bp_usXdGpio,                .u.us = BP_GPIO_15_AH},
  {bp_usXdGpioInitValue,       .u.us = 0},
  {bp_cpXdGpioInfo,            .u.cp = "DSP2_SGMII_SEL"},
  {bp_cpXdGpioInfoValue0,      .u.cp = "bypass dsp2 sgmii"},
  {bp_cpXdGpioInfoValue1,      .u.cp = "connect dsp2 sgmii"},
  {bp_last}
};

static bp_elem_t g_bcm965200dpf2_co[] = {
  {bp_cpBoardId,               .u.cp = "965200DPF2_CO"},
  {bp_cpComment,               .u.cp = ""},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm965200f_cpe},
  {bp_last}
};

static bp_elem_t g_bcm965200dpf2_cpe[] = {
  {bp_cpBoardId,               .u.cp = "965200DPF2_CPE"},
  {bp_cpComment,               .u.cp = ""},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm965200f_cpe},
  {bp_last}
};

static bp_elem_t g_bcm968380fhgu_pg[] = {
  {bp_cpBoardId,               .u.cp = "968380FHGU_PG"},  
  {bp_ulOpticalWan,            .u.ul = BP_OPTICAL_WAN_GPON},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_72_AL},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_47_AL},
  {bp_usExtIntrWifiOnOff ,     .u.us = BP_EXT_INTR_3},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_71_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_4_AL},
  {bp_usGpioVoip1Led,          .u.us = BP_LED_2_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_10_AL},
  {bp_usGpioLedGpon,           .u.us = BP_LED_5_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_15_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_LED_6_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_17_AL},
  {bp_usGpioLedOpticalLinkFail,.u.us = BP_LED_0_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_33_AL},
  {bp_usGpioLedUSB,            .u.us = BP_GPIO_53_AL},
  {bp_ucPhyType0,              .u.uc = BP_ENET_INTERNAL_PHY},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = 0x01 | MAC_IF_MII},
  {bp_ulPhyId1,                .u.ul = 0x02 | MAC_IF_MII},
  {bp_ulPhyId2,                .u.ul = 0x03 | MAC_IF_MII},
  {bp_ulPhyId3,                .u.ul = 0x04 | MAC_IF_MII},
  {bp_ulSimInterfaces,         .u.us = BP_SIMCARD_GROUPA},
  {bp_ulSlicInterfaces,        .u.us = BP_SLIC_GROUPD},
  {bp_InvSerdesRxPol,          .u.us = pmd_polarity_invert},
  {bp_InvSerdesTxPol,          .u.us = pmd_polarity_invert},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0}, 
  {bp_usRogueOnuEn,            .u.us = 1},
  {bp_usGpioPonTxEn,           .u.us = BP_GPIO_13_AH}, 
  {bp_usGpioPonRxEn,           .u.us = BP_GPIO_13_AH},
  /*{bp_usPmdMACEwakeEn,     .u.us = 1}, */
  {bp_usExtIntrPmdAlarm,       .u.us = BP_EXT_INTR_2 | BP_EXT_INTR_TYPE_IRQ_HIGH_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_61_AH},
  {bp_usGpioPmdReset,          .u.us = BP_GPIO_35_AL},
  {bp_pmdFunc,                 .u.us = BP_PMD_APD_REG_ENABLED | BP_PMD_APD_TYPE_FLYBACK},
  {bp_last}
};

static bp_elem_t g_bcm968380fhgu_dvt[] = {
  {bp_cpBoardId,               .u.cp = "968380FHGU_DVT"},  
  {bp_usRogueOnuEn,            .u.us = 0},
  {bp_usExtIntrPmdAlarm,       .u.us = BP_EXT_INTR_2},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_51_AH},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm968380fhgu_pg},
  {bp_last}
};

//BCM968381GREF board.
static bp_elem_t g_bcm968381gref[] = {
  {bp_cpBoardId,               .u.cp = "968381GREF"},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm968385gsp},
  {bp_last}
};

static bp_elem_t g_bcm968380moca[] = {
  {bp_cpBoardId,               .u.cp = "968380MOCA"},
  {bp_ulOpticalWan,            .u.ul = BP_OPTICAL_WAN_GPON},
  {bp_ulSlicInterfaces,        .u.us = BP_SLIC_GROUPD},
#if !defined(_CFE_)
  {bp_usMocaType0,             .u.us = BP_MOCA_TYPE_LAN},
  {bp_usMocaRfBand,            .u.us = BP_MOCA_RF_BAND_D_HIGH},
  {bp_pMocaInit,               .u.ptr = (void*)moca6802InitSeq},
#endif
  {bp_usExtIntrMocaHostIntr,   .u.us = BP_EXT_INTR_1},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_7_AL},
  {bp_usExtIntrMocaSBIntr0,    .u.us = BP_EXT_INTR_2},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_10_AL},
  {bp_usExtIntrMocaSBIntr1,    .u.us = BP_EXT_INTR_3},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_11_AL},
  {bp_usGpioSpiSlaveReset,     .u.us = BP_GPIO_4_AL},
  {bp_usSpiSlaveBusNum,        .u.us = HS_SPI_BUS_NUM},
  {bp_usSpiSlaveSelectNum,     .u.us = 1},
  {bp_usSpiSlaveMode,          .u.us = SPI_MODE_3},
  {bp_ulSpiSlaveCtrlState,     .u.ul = SPI_CONTROLLER_STATE_GATE_CLK_SSOFF},
  {bp_ulSpiSlaveMaxFreq,       .u.ul = 12500000},
  {bp_usExtIntrTrplxrTxFail,   .u.us = BP_EXT_INTR_4},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_12_AL},
  {bp_usExtIntrTrplxrSd,       .u.us = BP_EXT_INTR_5 | BP_EXT_INTR_TYPE_IRQ_HIGH_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_13_AH},
  {bp_usTxLaserOnOutN,         .u.us = 1},
  {bp_usGpioPonReset,          .u.us = BP_GPIO_14_AH},
  {bp_usGpioPonTxEn,           .u.us = BP_GPIO_16_AH},
  {bp_usGpioPonRxEn,           .u.us = BP_GPIO_16_AH},
  {bp_usSerialLEDMuxSel,       .u.us = BP_SERIAL_LED_MUX_GROUPB},
  {bp_usGpioLedWanData,        .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioLedGpon,           .u.us = BP_SERIAL_GPIO_9_AL},
  {bp_usGpioLedOpticalLinkFail, .u.us = BP_SERIAL_GPIO_10_AL},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_11_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_12_AL},
  {bp_usGpioLedUSB,            .u.us = BP_SERIAL_GPIO_13_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_15_AL},
  {bp_ucPhyType0,              .u.uc = BP_ENET_INTERNAL_PHY},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = 0x01 | MAC_IF_MII},
  {bp_ulPhyId1,                .u.ul = 0x02 | MAC_IF_MII},
  {bp_ulPhyId2,                .u.ul = 0x03 | MAC_IF_MII},
  {bp_ulPhyId3,                .u.ul = 0x04 | MAC_IF_MII},
  {bp_ulPhyId4,                .u.ul = RGMII_DIRECT},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_TX_INTERNAL_DELAY},
  {bp_usPhyConnType,           .u.us = PHY_CONN_TYPE_MOCA}, 
  {bp_ucPhyDevName,            .u.cp = "moca%d"},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0}, 
  {bp_last}
};

static bp_elem_t g_bcm968380lte[] = {
  {bp_cpBoardId,               .u.cp = "968380LTE"},
  {bp_ulOpticalWan,            .u.ul = BP_OPTICAL_WAN_GPON},
  {bp_ulSlicInterfaces,        .u.us = BP_SLIC_GROUPD},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_4_AL},
  {bp_usTsync1pps,             .u.us = BP_PIN_TSYNC_1PPS_6},
  {bp_usGpio1ppsStable,        .u.us = BP_GPIO_7_AL},
  {bp_usGpioLedGpon,           .u.us = BP_GPIO_8_AL},
  {bp_usGpioVoip1Led,          .u.us = BP_LED_2_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_10_AL},
  {bp_usGpioLedUSB,            .u.us = BP_LED_3_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_11_AL},
  {bp_usGpioPonTxEn,           .u.us = BP_GPIO_13_AH},
  {bp_usGpioPonRxEn,           .u.us = BP_GPIO_13_AH},
  {bp_usWanNco10MClk,          .u.us = 1},
  {bp_usGpioVoip2Led,          .u.us = BP_LED_6_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_17_AL},
  {bp_usGpioLteReset,          .u.us = BP_GPIO_18_AL},
  {bp_usGpioLedOpticalLinkFail, .u.us = BP_LED_0_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_33_AL},
  {bp_usGpioLedWanData,        .u.us = BP_LED_1_AL},
  {bp_usPinMux,                .u.us = BP_GPIO_34_AL},
  {bp_usRogueOnuEn,            .u.us = 1},
  {bp_usTrxSignalDetect,       .u.us = 1},
  {bp_usGpioStrapTxEn,         .u.us = BP_GPIO_58_AL},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_0},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_57_AL},
#if 0 /* Gil for Gorden */
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_1},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_72_AL},
#endif 
  {bp_usExtIntrWifiOnOff,      .u.us = BP_EXT_INTR_2},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_59_AL},
  {bp_usExtIntrLTE,            .u.us = BP_EXT_INTR_3},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_61_AL},
  {bp_ucPhyType0,              .u.uc = BP_ENET_INTERNAL_PHY},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = 0x01 | MAC_IF_MII},
  {bp_ulPhyId1,                .u.ul = 0x02 | MAC_IF_MII},
  {bp_ulPhyId2,                .u.ul = 0x03 | MAC_IF_MII},
  {bp_ulPhyId3,                .u.ul = 0x04 | MAC_IF_MII},
  {bp_ulPhyId4,                .u.ul = RGMII_DIRECT},
  {bp_usGpioUart2Sdin,         .u.us = BP_GPIO_14_AH}, // uncomment to enable second UART
  {bp_usGpioUart2Sdout,        .u.us = BP_GPIO_15_AH}, // uncomment to enable second UART 
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0}, 
  {bp_last}
};

static bp_elem_t g_bcm968380gwan[] = {
  {bp_cpBoardId,               .u.cp = "968380GWAN"},
  {bp_ulOpticalWan,            .u.ul = BP_OPTICAL_WAN_GPON},
  {bp_usMiiInterfaceEn,        .u.us = 1},
  {bp_usSerialLEDMuxSel,       .u.us = BP_SERIAL_LED_MUX_GROUPC},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_10_AL},
  {bp_usExtIntrWifiOnOff,      .u.us = BP_EXT_INTR_1},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_53_AL},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_2},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_18_AL},
  {bp_usGpioLedUSB,            .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_usGpioLedOpticalLinkFail,.u.us = BP_SERIAL_GPIO_5_AL},
  {bp_usGpioLedWanData,        .u.us = BP_SERIAL_GPIO_6_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_7_AL},
  {bp_ucPhyType0,              .u.uc = BP_ENET_INTERNAL_PHY},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x3f},
  {bp_ulPhyId0,                .u.ul = 0x01 | MAC_IF_GMII},
  {bp_ulPhyId1,                .u.ul = 0x02 | MAC_IF_GMII},
  {bp_ulPhyId2,                .u.ul = 0x03 | MAC_IF_GMII},
  {bp_ulPhyId3,                .u.ul = 0x04 | MAC_IF_GMII},
  {bp_ulPhyId4,                .u.ul = 0x03 | MAC_IF_RGMII | PHY_EXTERNAL | PHY_INTEGRATED_VALID},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_TX_INTERNAL_DELAY},
  {bp_ulPhyId5,                .u.ul = 0x01 | BCM_WAN_PORT | PHYCFG_VALID | PHY_EXTERNAL | MAC_IF_SERDES}, /* WAN interface */
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_DETECT | PORT_FLAG_WAN_ONLY},
  {bp_ulSimInterfaces,         .u.us = BP_SIMCARD_GROUPA},
  {bp_ulSlicInterfaces,        .u.us = BP_SLIC_GROUPD},
  {bp_usGpioPonTxEn,           .u.us = BP_GPIO_13_AH},
  {bp_usGpioPonRxEn,           .u.us = BP_GPIO_13_AH},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usRogueOnuEn,            .u.us = 1},
  {bp_usGpioWanSignalDetected, .u.us = BP_GPIO_53_AH},
  {bp_last}
};

static bp_elem_t g_bcm968380gwan_opt[] = {
  {bp_cpBoardId,               .u.cp = "968380GWAN_OPT"},
  {bp_ucPhyType0,              .u.uc = BP_ENET_INTERNAL_PHY},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x3f},
  {bp_ulPhyId0,                .u.ul = 0x01 | MAC_IF_GMII},
  {bp_ulPhyId1,                .u.ul = 0x02 | MAC_IF_GMII},
  {bp_ulPhyId2,                .u.ul = 0x03 | MAC_IF_GMII},
  {bp_ulPhyId3,                .u.ul = 0x04 | MAC_IF_GMII},
  {bp_ulPhyId4,                .u.ul = 0x03 | MAC_IF_RGMII | PHY_EXTERNAL | PHY_INTEGRATED_VALID},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_TX_INTERNAL_DELAY},
  {bp_ulPhyId5,                .u.ul = 0x02 | BCM_WAN_PORT | MAC_IF_SERDES}, /* WAN interface */
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_DETECT | PORT_FLAG_WAN_ONLY},
  {bp_usAePolarity,            .u.us = 1},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm968380gwan},
  {bp_last}
};

static bp_elem_t g_bcm968389wlnvb[] = {
  {bp_cpBoardId,               .u.cp = "968389WLNVB"},
  {bp_ulOpticalWan,            .u.ul = BP_OPTICAL_WAN_GPON},
  {bp_usMiiInterfaceEn,        .u.us = 1},
  {bp_usSerialLEDMuxSel,       .u.us = BP_SERIAL_LED_MUX_GROUPC},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_10_AL},
  {bp_usExtIntrWifiOnOff,      .u.us = BP_EXT_INTR_1},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_53_AL},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_2},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_18_AL},
  {bp_usGpioLedUSB,            .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_usGpioLedOpticalLinkFail,.u.us = BP_SERIAL_GPIO_5_AL},
  {bp_usGpioLedWanData,        .u.us = BP_SERIAL_GPIO_6_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_7_AL},
  {bp_ucPhyType0,              .u.uc = BP_ENET_INTERNAL_PHY},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = 0x01 | MAC_IF_GMII},
  {bp_ulPhyId1,                .u.ul = 0x02 | MAC_IF_GMII},
  {bp_ulPhyId2,                .u.ul = 0x03 | MAC_IF_GMII},
  {bp_ulPhyId3,                .u.ul = 0x04 | MAC_IF_GMII},
  {bp_ulPhyId4,                .u.ul = 0x01 | BCM_WAN_PORT | PHYCFG_VALID | PHY_EXTERNAL | MAC_IF_SERDES}, /* WAN interface */
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_DETECT | PORT_FLAG_WAN_ONLY},
  {bp_ulSimInterfaces,         .u.us = BP_SIMCARD_GROUPA},
  {bp_ulSlicInterfaces,        .u.us = BP_SLIC_GROUPD},
  {bp_usGpioPonTxEn,           .u.us = BP_GPIO_13_AH},
  {bp_usGpioPonRxEn,           .u.us = BP_GPIO_13_AH},
  {bp_ucDspType0,              .u.uc = BP_VOIP_NO_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usRogueOnuEn,            .u.us = 1},
  {bp_usGpioWanSignalDetected, .u.us = BP_GPIO_53_AH},
  {bp_last}
};

static bp_elem_t g_bcm968385pgsp[] = {
  {bp_cpBoardId,               .u.cp = "968385PGSP"},
  {bp_InvSerdesRxPol,          .u.us = pmd_polarity_invert},
  {bp_InvSerdesTxPol,          .u.us = pmd_polarity_invert},
  {bp_usPmdMACEwakeEn,         .u.us = BP_GPIO_10_AH},
  {bp_usExtIntrPmdAlarm,       .u.us = BP_EXT_INTR_2 | BP_EXT_INTR_TYPE_IRQ_HIGH_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_61_AH},
  {bp_usExtIntrWanSignalDetected,     .u.us = BP_EXT_INTR_3 | BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_11_AL},
  {bp_usGpioPmdReset,          .u.us = BP_GPIO_35_AL},
  {bp_pmdFunc,                 .u.us = BP_PMD_APD_REG_ENABLED | BP_PMD_APD_TYPE_FLYBACK},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm968385gsp},
  {bp_last}
};


bp_elem_t * g_BoardParms[] = {g_bcm968380fhgu, g_bcm968380fhgu_si, g_bcm968380ffhg, g_bcm968380gerg, g_bcm968380mgeg, g_bcm968380gerg_si,
    g_bcm968380fsv_g, g_bcm968380sv_g, g_bcm968380fggu, g_bcm968380fggu_tri, g_bcm968385sfu, g_bcm968385sfu_si, g_bcm968385gsp, g_bcm968385sv_g,
    g_bcm968380fttdps, g_bcm968380dp2, g_bcm965200f_cpe, g_bcm965200f_co, g_bcm965200dpf, g_bcm965200dpf2_co, g_bcm965200dpf2_cpe, g_bcm968380fhgu_pg,
    g_bcm968380fhgu_dvt, g_bcm968381gref, g_bcm968381sv_g, g_bcm968380moca, g_bcm968380lte, g_bcm968380gwan, g_bcm968380gwan_opt, g_bcm968385pgsp, g_bcm968389wlnvb, 0};



