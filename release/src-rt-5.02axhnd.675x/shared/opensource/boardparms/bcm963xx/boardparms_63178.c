#include "bp_defs.h"
#include "boardparms.h"
#include "bcmSpiRes.h"
extern BpCmdElem moca6802InitSeq[];

#define BCM963178_PHY_BASE             0x8
#define BP_DDR_63178_DEFAULT           BP_DDR_VTT_DIS_NOTERM

static bp_elem_t g_bcm963178sv[] = {
  {bp_cpBoardId,               .u.cp = "963178SV"},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND}, 
  {bp_usSerialLedData,         .u.us = BP_GPIO_26_AH},
  {bp_usSerialLedClk,          .u.us = BP_GPIO_27_AH},
  {bp_usSerialLedMask,         .u.us = BP_GPIO_28_AH},
  {bp_usButtonIdx,             .u.us = 0},
  {  bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {  bp_usGpio_Intr,           .u.us = BP_GPIO_0_AL},
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {      bp_ulButtonActionParm,  .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},  
  {bp_usButtonIdx,             .u.us = 1},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_1|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {  bp_usGpio_Intr,           .u.us = BP_GPIO_1_AL},
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },

  {bp_usGpioBitbangI2cSda,     .u.us = BP_GPIO_3_AH},
  {bp_usGpioBitbangI2cScl,     .u.us = BP_GPIO_4_AH},
  {bp_usI2sSclk,               .u.us = BP_GPIO_6_AH},
  {bp_usI2sLrck,               .u.us = BP_GPIO_7_AH},
  {bp_usI2sRxSdata,            .u.us = BP_GPIO_8_AH},
  {bp_usI2sTxSdata,            .u.us = BP_GPIO_9_AH},
  {bp_usI2sMclk,               .u.us = BP_GPIO_10_AH},
  {bp_usPcmSdin,               .u.us = BP_GPIO_13_AH},
  {bp_usPcmSdout,              .u.us = BP_GPIO_14_AH},
  {bp_usPcmClk,                .u.us = BP_GPIO_15_AH},
  {bp_usPcmFs,                 .u.us = BP_GPIO_16_AH},
  {bp_usGpioBtWake,            .u.us = BP_GPIO_24_AH},
  {bp_usGpioBtReset,           .u.us = BP_GPIO_29_AH},
  {bp_usSpiSlaveSelectNum,     .u.us = 1},
  {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_80_AL},

  /* wlan fem_ctrl reserved signals*/
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_34_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_35_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_36_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_37_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_38_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_39_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_40_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_41_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_42_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_43_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_44_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_45_AH},

  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_28_AH}, /*wlan0 led? */
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_26_AH},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_27_AH},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_29_AH},

  {bp_usGphyBaseAddress,       .u.us = BCM963178_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x3f},
  {bp_ulPhyId0,                .u.ul = (BCM963178_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963178_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963178_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963178_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId4,                .u.ul = (BCM963178_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId5,                .u.ul =  0x18 | PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},

  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 0},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_0_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_15_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 1},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_2_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_16_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 2},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 2},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_4_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_17_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 3},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 3},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_6_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_18_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 4},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 4},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_8_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_19_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 5},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_30_AH},
  {bp_usGpioLedWanAct,         .u.us = BP_SERIAL_GPIO_23_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_70},
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_17_AH}, // Line Driver 0 = "Int"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_18_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_19_AH},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 6}, /* HS_UART for BT */
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_UART},
  {bp_usPortNum,               .u.us = 1},
  {bp_usUartSdin,              .u.us = BP_GPIO_21_AH},
  {bp_usUartSdout,             .u.us = BP_GPIO_20_AH},
  {bp_usUartCts,               .u.us = BP_GPIO_22_AH},
  {bp_usUartRts,               .u.us = BP_GPIO_12_AH},
  {bp_usIntfEnd},
  {bp_usUsbPwrOn0,             .u.us = BP_GPIO_84_AL},
  {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_83_AL},
  {bp_usUsbPwrOn1,             .u.us = BP_GPIO_86_AL},
  {bp_usUsbPwrFlt1,            .u.us = BP_GPIO_85_AL},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_63178_DEFAULT},
  {bp_last}
};

static bp_elem_t g_bcm963178sv_p200[] = {
  {bp_cpBoardId,               .u.cp = "963178SV_P200"},

  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_30_AH}, /* color led */
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_29_AH}, /* color led */
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_28_AH}, /* color led */
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_27_AH}, /* color led */
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_26_AH}, /* color led */
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_14_AH}, /* color led */
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_13_AH}, /* color led */
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_12_AH}, /* color led */
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_11_AH}, /* color led */
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_10_AH}, /* color led */
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_9_AH},  /* color led */
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_7_AH},  /* color led */
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_5_AH},  /* color led */
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_3_AH},  /* color led */
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_1_AH},  /* color led */
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_20_AH}, /* wlan0 led */

  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_25_AH},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_24_AH},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_21_AH},

  {bp_usIntfId,                .u.us = 5},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_22_AH},
  {bp_usGpioLedWanAct,         .u.us = BP_SERIAL_GPIO_23_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_70},
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_17_AH}, // Line Driver 0 = "Int"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_18_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_19_AH},
  {bp_usIntfEnd},

  {bp_elemTemplate,            .u.bp_elemp = g_bcm963178sv},
  {bp_last}
};

static bp_elem_t g_bcm963178ref1[] = {
  {bp_cpBoardId,               .u.cp = "963178REF1"},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND},
  {bp_usSerialLedData,         .u.us = BP_GPIO_26_AH},
  {bp_usSerialLedClk,          .u.us = BP_GPIO_27_AH},
  {bp_usSerialLedMask,         .u.us = BP_GPIO_28_AH},
  {bp_usButtonIdx,             .u.us = 0},
  {  bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {  bp_usGpio_Intr,           .u.us = BP_GPIO_0_AL},
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {      bp_ulButtonActionParm,  .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},  
  {bp_usButtonIdx,             .u.us = 1},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_1|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {  bp_usGpio_Intr,           .u.us = BP_GPIO_1_AL},
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },

  /* wlan bluetooth co-existance reserved pins */
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_30_AL},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_31_AL},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_32_AL},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_33_AL},

  {bp_usGpioLedWL0Act,         .u.us = BP_SERIAL_GPIO_28_AH}, /* wlan0 led */
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_26_AH},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_27_AH},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_29_AH},
#if !defined(_CFE_)
  {bp_usVregSync,              .u.us = BP_GPIO_2_AH},
#endif
  {bp_usGpioBitbangI2cSda,     .u.us = BP_GPIO_3_AH},
  {bp_usGpioBitbangI2cScl,     .u.us = BP_GPIO_4_AH},
  {bp_usI2sSclk,               .u.us = BP_GPIO_6_AH},
  {bp_usI2sLrck,               .u.us = BP_GPIO_7_AH},
  {bp_usI2sRxSdata,            .u.us = BP_GPIO_8_AH},
  {bp_usI2sTxSdata,            .u.us = BP_GPIO_9_AH},
  {bp_usI2sMclk,               .u.us = BP_GPIO_10_AH},
  {bp_usPcmSdin,               .u.us = BP_GPIO_13_AH},
  {bp_usPcmSdout,              .u.us = BP_GPIO_14_AH},
  {bp_usPcmClk,                .u.us = BP_GPIO_15_AH},
  {bp_usPcmFs,                 .u.us = BP_GPIO_16_AH},
  {bp_usSpiSlaveSelectNum,     .u.us = 1}, /* define the SPI select for voice */
  {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_80_AL},

  {bp_usGphyBaseAddress,       .u.us = BCM963178_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = (BCM963178_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963178_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963178_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963178_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId4,                .u.ul = (BCM963178_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},

  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 0},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_0_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_15_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 1},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_2_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_16_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 2},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 2},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_4_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_17_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 3},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 3},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_6_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_18_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 4},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 4},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_8_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_19_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},

  {bp_usIntfId,                .u.us = 5},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_30_AH},
  {bp_usGpioLedWanAct,         .u.us = BP_SERIAL_GPIO_23_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_70},
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_17_AH}, // Line Driver 0 = "Int"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_18_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_19_AH},
  {bp_usIntfEnd},

  {bp_usIntfId,                .u.us = 6}, /* HS_UART for BT */
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_UART},
  {bp_usPortNum,               .u.us = 1},
  {bp_usUartSdin,              .u.us = BP_GPIO_21_AH},
  {bp_usUartSdout,             .u.us = BP_GPIO_20_AH},
  {bp_usUartCts,               .u.us = BP_GPIO_22_AH},
  {bp_usUartRts,               .u.us = BP_GPIO_12_AH},
  {bp_usIntfEnd},
  {bp_usGpioBtWake,            .u.us = BP_GPIO_24_AH},
  {bp_usGpioBtReset,           .u.us = BP_GPIO_29_AH},
  {bp_usUsbPwrOn1,             .u.us = BP_GPIO_86_AL},
  {bp_usUsbPwrFlt1,            .u.us = BP_GPIO_85_AL},
  {bp_usUsbPwrOn0,             .u.us = BP_GPIO_84_AL},
  {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_83_AL},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_1024MB| BP_DDR_DEVICE_WIDTH_8 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_63178_DEFAULT},
  {bp_last}
};


static bp_elem_t g_bcm963178ref1_epa[] = {
  {bp_cpBoardId,               .u.cp = "963178REF1_EPA"},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963178ref1},
  {bp_last}
};

static bp_elem_t g_bcm963178ref3[] = {
  {bp_cpBoardId,               .u.cp = "963178REF3"},
  {bp_usButtonIdx,             .u.us = 0},
  {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_0_AL},
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {bp_ulButtonActionParm,      .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},
  {bp_usButtonIdx,             .u.us = 1},
  {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_1|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_1_AL},
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },

#if !defined(_CFE_)
  {bp_usVregSync,              .u.us = BP_GPIO_2_AH},
#endif
  {bp_usGpioBitbangI2cSda,     .u.us = BP_GPIO_72_AH},
  {bp_usGpioBitbangI2cScl,     .u.us = BP_GPIO_73_AH},
  {bp_usI2sSclk,               .u.us = BP_GPIO_67_AH},
  {bp_usI2sLrck,               .u.us = BP_GPIO_68_AH},
  {bp_usI2sRxSdata,            .u.us = BP_GPIO_69_AH},
  {bp_usI2sTxSdata,            .u.us = BP_GPIO_70_AH},
  {bp_usI2sMclk,               .u.us = BP_GPIO_71_AH},
  {bp_usPcmSdin,               .u.us = BP_GPIO_13_AH},
  {bp_usPcmSdout,              .u.us = BP_GPIO_14_AH},
  {bp_usPcmClk,                .u.us = BP_GPIO_15_AH},
  {bp_usPcmFs,                 .u.us = BP_GPIO_16_AH},

  /* wlan fem_ctrl reserved signals*/
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_34_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_35_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_36_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_40_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_41_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_42_AH},

  {bp_usGpioVoip1Led,          .u.us = BP_GPIO_51_AL},
  {bp_ulLedChannelId,          .u.ul = 27},
  {bp_usGpioVoip2Led,          .u.us = BP_GPIO_52_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_53_AL},
  {bp_usGpioLedWL0Act,         .u.us = BP_GPIO_54_AL | BP_LED_USE_GPIO}, /* wlan0 led */
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_11_AL},

  {bp_usUsbPwrOn0,             .u.us = BP_GPIO_84_AL},
  {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_83_AL},

  {bp_usGphyBaseAddress,       .u.us = BCM963178_PHY_BASE},  // use phy addressses on SF2 with base address 0x8 
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH}, 
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch 
  {bp_ulPortMap,               .u.ul = 0x1f}, 
  {bp_ulPhyId0,                .u.ul = (BCM963178_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)}, 
  {bp_ulPhyId1,                .u.ul = (BCM963178_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)}, 
  {bp_ulPhyId2,                .u.ul = (BCM963178_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)}, 
  {bp_ulPhyId3,                .u.ul = (BCM963178_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)}, 
  {bp_ulPhyId4,                .u.ul = (BCM963178_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)}, 
 
  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 0},
  {bp_usNetLed0,               .u.us = BP_GPIO_55_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed1,               .u.us = BP_GPIO_22_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_GPIO_24_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 1},
  {bp_usNetLed0,               .u.us = BP_GPIO_7_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed1,               .u.us = BP_GPIO_6_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_GPIO_8_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 2},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 2},
  {bp_usNetLed0,               .u.us = BP_GPIO_5_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed1,               .u.us = BP_GPIO_9_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_GPIO_29_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 3},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 3},
  {bp_usNetLed0,               .u.us = BP_GPIO_56_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed1,               .u.us = BP_GPIO_20_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_GPIO_10_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd}, 
  {bp_usIntfId,                .u.us = 4},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 4},
  {bp_usNetLed0,               .u.us = BP_GPIO_3_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed1,               .u.us = BP_GPIO_4_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_GPIO_21_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},

  {bp_usIntfId,                .u.us = 5},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_GPIO_25_AL},
  {bp_ulLedChannelId,          .u.ul = 24},
  {bp_usGpioLedWanAct,         .u.us = BP_GPIO_49_AL},
  {bp_usGpioLedWanErr,         .u.us = BP_GPIO_50_AL},
  {bp_ulLedChannelId,          .u.ul = 31},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_70},
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_17_AH}, // Line Driver 0 = "Int"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_18_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_19_AH},
  {bp_usIntfEnd},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_63178_DEFAULT},
  {bp_last}
};

static bp_elem_t g_bcm963178ref2_base[] = {
  {bp_cpBoardId,               .u.cp = "963178REF2BASE"},
  {bp_usButtonIdx,             .u.us = 0},
  {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_0_AL},
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {bp_ulButtonActionParm,      .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},
  {bp_usButtonIdx,             .u.us = 1},
  {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_1|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_1_AL},
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },

#if !defined(_CFE_)
  {bp_usVregSync,              .u.us = BP_GPIO_2_AH},
#endif
  {bp_usI2sSclk,               .u.us = BP_GPIO_67_AH},
  {bp_usI2sLrck,               .u.us = BP_GPIO_68_AH},
  {bp_usI2sRxSdata,            .u.us = BP_GPIO_69_AH},
  {bp_usI2sTxSdata,            .u.us = BP_GPIO_70_AH},
  {bp_usI2sMclk,               .u.us = BP_GPIO_71_AH},
  {bp_usPcmSdin,               .u.us = BP_GPIO_13_AH},
  {bp_usPcmSdout,              .u.us = BP_GPIO_14_AH},
  {bp_usPcmClk,                .u.us = BP_GPIO_15_AH},
  {bp_usPcmFs,                 .u.us = BP_GPIO_16_AH},

  {bp_usGpioVoip1Led,          .u.us = BP_GPIO_51_AL},
  {bp_ulLedChannelId,          .u.ul = 27},
  {bp_usGpioVoip2Led,          .u.us = BP_GPIO_52_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_53_AL},
  {bp_usGpioLedWL0Act,         .u.us = BP_GPIO_54_AL | BP_LED_USE_GPIO}, /* wlan0 led */
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_11_AL},
  {bp_usGpioWireless0Disable,  .u.us = BP_GPIO_27_AL},

  {bp_usUsbPwrOn0,             .u.us = BP_GPIO_84_AH},
  {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_83_AL},

  {bp_usGphyBaseAddress,       .u.us = BCM963178_PHY_BASE},  // use phy addressses on SF2 with base address 0x8 
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH}, 
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch 
  {bp_ulPortMap,               .u.ul = 0x1f}, 
  {bp_ulPhyId0,                .u.ul = (BCM963178_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)}, 
  {bp_ulPhyId1,                .u.ul = (BCM963178_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)}, 
  {bp_ulPhyId2,                .u.ul = (BCM963178_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)}, 
  {bp_ulPhyId3,                .u.ul = (BCM963178_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)}, 
  {bp_ulPhyId4,                .u.ul = (BCM963178_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)}, 
 
  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 0},
  {bp_usNetLed0,               .u.us = BP_GPIO_55_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed1,               .u.us = BP_GPIO_22_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_GPIO_24_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 1},
  {bp_usNetLed0,               .u.us = BP_GPIO_7_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed1,               .u.us = BP_GPIO_6_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_GPIO_8_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 2},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 2},
  {bp_usNetLed0,               .u.us = BP_GPIO_5_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed1,               .u.us = BP_GPIO_9_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_GPIO_29_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 3},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 3},
  {bp_usNetLed0,               .u.us = BP_GPIO_56_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed1,               .u.us = BP_GPIO_20_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_GPIO_10_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd}, 
  {bp_usIntfId,                .u.us = 4},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 4},
  {bp_usNetLed0,               .u.us = BP_GPIO_3_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed1,               .u.us = BP_GPIO_4_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_GPIO_21_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},

  {bp_usIntfId,                .u.us = 5},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_GPIO_25_AL},
  {bp_ulLedChannelId,          .u.ul = 24},
  {bp_usGpioLedWanAct,         .u.us = BP_GPIO_49_AL},
  {bp_usGpioLedWanErr,         .u.us = BP_GPIO_50_AL},
  {bp_ulLedChannelId,          .u.ul = 31},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_70},
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_17_AH}, // Line Driver 0 = "Int"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_18_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_19_AH},
  {bp_usIntfEnd},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_63178_DEFAULT},
  {bp_last}
};

static bp_elem_t g_bcm963178ref2[] = {
  {bp_cpBoardId,               .u.cp = "963178REF2"},

  /* wlan fem_ctrl reserved signals*/
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_35_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_38_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_41_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_44_AH},

  {bp_elemTemplate,            .u.bp_elemp = g_bcm963178ref2_base},
  {bp_last}
};

static bp_elem_t g_bcm963178ref2_p200[] = {
  {bp_cpBoardId,               .u.cp = "963178REF2_P200"},

  /* wlan fem_ctrl reserved signals*/
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_35_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_37_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_41_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_43_AH},

  {bp_elemTemplate,            .u.bp_elemp = g_bcm963178ref2_base},
  {bp_last}
};

static bp_elem_t g_bcm963178ref2_p300[] = {
  {bp_cpBoardId,               .u.cp = "963178REF2_P300"},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963178ref2_p200},
  {bp_last}
};

#if 0
static bp_elem_t g_bcm963178ref6[] = {
  {bp_cpBoardId,               .u.cp = "963178REF6"},

  /* wlan fem_ctrl reserved signals*/
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_34_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_36_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_37_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_40_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_42_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_43_AH},

  {bp_elemTemplate,            .u.bp_elemp = g_bcm963178ref2_base},
  {bp_last}
};
#else
static bp_elem_t g_bcm963178ref6[] = {
  {bp_cpBoardId,               .u.cp = "963178REF6"},
  {bp_usButtonIdx,             .u.us = 0},
  {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_0_AL},
#if defined(_CFE_)
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {bp_ulButtonActionParm,      .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},
#endif
  {bp_usButtonIdx,             .u.us = 1},
  {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_1|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_1_AL},
#if defined(_CFE_)
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },
#endif

  /* wlan fem_ctrl reserved signals*/
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_34_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_35_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_36_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_40_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_42_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_44_AH},

  {bp_usGpioLedReserved,       .u.us = BP_GPIO_17_AL},	// WAN RED LED
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_23_AL},

  {bp_usGpioLedReserved,       .u.us = BP_GPIO_2_AH},	// LED Group2 Red
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_3_AH},	// LED Group2 Green
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_4_AH},	// LED Group2 Blue
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_5_AH},	// LED Group1 Blue
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_6_AH},	// LED Group1 Green
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_8_AH},	// LED Group4 Red
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_11_AH},	// LED Group3 Red
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_13_AH},	// LED Group3 Green
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_15_AH},	// LED Group5 Red
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_10_AH},	// LED Group5 Green
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_22_AH},	// LED Group1 Red
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_24_AH},	// LED Group3 Blue
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_25_AH},	// LED Group4 Blue
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_28_AH},	// LED Group5 Blue
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_29_AH},	// LED Group4 Green

  {bp_usGpioLedWL0Act,         .u.us = BP_GPIO_31_AL | BP_LED_USE_GPIO},  /* WLAN_2G_ACT, must use BP_LED_USE_GPIO */
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_84_AL},	// USB power
  {bp_usButtonIdx,             .u.us = 2},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_7_AL},	// 4T4R
  {bp_usButtonIdx,             .u.us = 3},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_9_AL},	// ePA

  {bp_usGphyBaseAddress,       .u.us = BCM963178_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = (BCM963178_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963178_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963178_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963178_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId4,                .u.ul = (BCM963178_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usIntfId,                .u.us = 4},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 4},
  {bp_usNetLed3,               .u.us = BP_GPIO_21_AL},	// WAN
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},

  {bp_usUsbPwrOn1,             .u.us = BP_GPIO_86_AL},
  {bp_usUsbPwrFlt1,            .u.us = BP_GPIO_85_AL},
  {bp_usUsbPwrOn0,             .u.us = BP_GPIO_84_AL},
  {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_83_AL},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_63178_DEFAULT},
  {bp_last}
};
#endif

#if 0
static bp_elem_t g_bcm963178ref5[] = {
  {bp_cpBoardId,               .u.cp = "963178REF5"},
  {bp_usButtonIdx,             .u.us = 0},
  {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_0_AL},
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {bp_ulButtonActionParm,      .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},
  {bp_usButtonIdx,             .u.us = 1},
  {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_1|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_1_AL},
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },

#if !defined(_CFE_)
  {bp_usVregSync,              .u.us = BP_GPIO_2_AH},
#endif
  {bp_usPcmSdin,               .u.us = BP_GPIO_13_AH},
  {bp_usPcmSdout,              .u.us = BP_GPIO_14_AH},
  {bp_usPcmClk,                .u.us = BP_GPIO_15_AH},
  {bp_usPcmFs,                 .u.us = BP_GPIO_16_AH},

  /* wlan fem_ctrl reserved signals*/
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_35_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_37_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_41_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_43_AH},

  {bp_usGpioVoip1Led,          .u.us = BP_GPIO_51_AL},
  {bp_ulLedChannelId,          .u.ul = 27},
  {bp_usGpioVoip2Led,          .u.us = BP_GPIO_52_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_53_AL},
  {bp_usGpioLedWL0Act,         .u.us = BP_GPIO_54_AL | BP_LED_USE_GPIO}, /* wlan0 led */

  {bp_usUsbPwrOn0,             .u.us = BP_GPIO_84_AL},
  {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_83_AL},
  {bp_usGpioWireless0Disable,  .u.us = BP_GPIO_11_AL},

  {bp_usGphyBaseAddress,       .u.us = BCM963178_PHY_BASE},  // use phy addressses on SF2 with base address 0x8 
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH}, 
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch 
  {bp_ulPortMap,               .u.ul = 0x1f}, 
  {bp_ulPhyId0,                .u.ul = (BCM963178_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)}, 
  {bp_ulPhyId1,                .u.ul = (BCM963178_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)}, 
  {bp_ulPhyId2,                .u.ul = (BCM963178_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)}, 
  {bp_ulPhyId3,                .u.ul = (BCM963178_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)}, 
  {bp_ulPhyId4,                .u.ul = (BCM963178_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)}, 
 
  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 0},
  {bp_usNetLed0,               .u.us = BP_GPIO_55_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed1,               .u.us = BP_GPIO_22_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_GPIO_24_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 1},
  {bp_usNetLed0,               .u.us = BP_GPIO_7_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed1,               .u.us = BP_GPIO_6_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_GPIO_8_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 2},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 2},
  {bp_usNetLed0,               .u.us = BP_GPIO_5_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed1,               .u.us = BP_GPIO_9_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_GPIO_29_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 3},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 3},
  {bp_usNetLed0,               .u.us = BP_GPIO_56_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed1,               .u.us = BP_GPIO_20_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_GPIO_10_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd}, 
  {bp_usIntfId,                .u.us = 4},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 4},
  {bp_usNetLed0,               .u.us = BP_GPIO_3_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_GPIO_21_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},

  {bp_usIntfId,                .u.us = 5},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_GPIO_25_AL},
  {bp_ulLedChannelId,          .u.ul = 24},
  {bp_usGpioLedWanAct,         .u.us = BP_GPIO_49_AL},
  {bp_usGpioLedWanErr,         .u.us = BP_GPIO_50_AL},
  {bp_ulLedChannelId,          .u.ul = 31},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_70},
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_17_AH}, // Line Driver 0 = "Int"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_18_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_19_AH},
  {bp_usIntfEnd},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_63178_DEFAULT},
  {bp_last}
};
#endif

static bp_elem_t g_bcm963178ref5[] = {
  {bp_cpBoardId,               .u.cp = "963178REF5"},
  {bp_usButtonIdx,             .u.us = 0},
  {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_0_AL},
#if defined(_CFE_)
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {bp_ulButtonActionParm,      .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},
#endif
  {bp_usButtonIdx,             .u.us = 1},
  {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_1|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_1_AL},
#if defined(_CFE_)
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },
#endif

  /* wlan fem_ctrl reserved signals*/
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_34_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_35_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_36_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_40_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_42_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_44_AH},

  {bp_usGpioLedReserved,       .u.us = BP_GPIO_30_AL},	// WAN RED LED
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_23_AL},

  {bp_usGpioLedReserved,       .u.us = BP_GPIO_2_AH},	// LED Group2 Red
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_3_AH},	// LED Group2 Green
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_4_AH},	// LED Group2 Blue
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_5_AH},	// LED Group1 Blue
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_6_AH},	// LED Group1 Green
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_8_AH},	// LED Group4 Red
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_11_AH},	// LED Group3 Red
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_13_AH},	// LED Group3 Green
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_22_AH},	// LED Group1 Red
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_24_AH},	// LED Group3 Blue
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_25_AH},	// LED Group4 Blue
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_29_AH},	// LED Group4 Green

  {bp_usGpioLedWL0Act,         .u.us = BP_GPIO_31_AL | BP_LED_USE_GPIO},  /* WLAN_2G_ACT, must use BP_LED_USE_GPIO */
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_84_AL},	// USB power
  {bp_usButtonIdx,             .u.us = 2},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_7_AL},	// 4T4R
  {bp_usButtonIdx,             .u.us = 3},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_9_AL},	// ePA
  {bp_usButtonIdx,             .u.us = 4},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_10_AL},	// LED button

  {bp_usGphyBaseAddress,       .u.us = BCM963178_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = (BCM963178_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963178_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963178_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963178_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId4,                .u.ul = (BCM963178_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usIntfId,                .u.us = 4},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 4},
  {bp_usNetLed3,               .u.us = BP_GPIO_21_AL},	// WAN
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},

  {bp_usUsbPwrOn1,             .u.us = BP_GPIO_86_AL},
  {bp_usUsbPwrFlt1,            .u.us = BP_GPIO_85_AL},
  {bp_usUsbPwrOn0,             .u.us = BP_GPIO_84_AL},
  {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_83_AL},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_63178_DEFAULT},
  {bp_last}
};

static bp_elem_t g_bcm963178rfdvt[] = {
  {bp_cpBoardId,               .u.cp = "963178RFDVT"},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND},
  {bp_usButtonIdx,             .u.us = 0},
  {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_0_AL},
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {bp_ulButtonActionParm,      .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},  
  {bp_usButtonIdx,             .u.us = 1},
  {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_1|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_1_AL},
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },

#if !defined(_CFE_)
  {bp_usVregSync,              .u.us = BP_GPIO_2_AH},
#endif

  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_3_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_4_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_19_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_20_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_21_AH},
  
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_29_AL},

  {bp_usGphyBaseAddress,       .u.us = BCM963178_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x03},
  {bp_ulPhyId0,                .u.ul = (BCM963178_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963178_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},

  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 0},
  {bp_usNetLed0,               .u.us = BP_GPIO_71_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed1,               .u.us = BP_GPIO_70_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_GPIO_72_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},

  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 1},
  {bp_usNetLed0,               .u.us = BP_GPIO_66_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed1,               .u.us = BP_GPIO_69_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_GPIO_60_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},

  {bp_usIntfId,                .u.us = 2},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanAct,         .u.us = BP_GPIO_8_AL},
  {bp_usGpioLedWanLink,        .u.us = BP_GPIO_30_AL},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_70},
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_17_AH}, // Line Driver 0 = "Int"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_18_AH},
  /* {bp_usGpioAFELDClk,          .u.us = BP_GPIO_19_AH},*/  //This is conflict with GPIO 19 for WL reserve pin
  {bp_usIntfEnd},

  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_63178_DEFAULT},
  {bp_last}
};

#ifdef BCM_MFG
static bp_elem_t g_bcm96750ref4[] = {
  {bp_cpBoardId,               .u.cp = "96750REF4"},
  {bp_usButtonIdx,             .u.us = 0},
  {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_0_AL},
#if defined(_CFE_)
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {bp_ulButtonActionParm,      .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},
#endif
  {bp_usButtonIdx,             .u.us = 1},
  {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_1|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_1_AL},
#if defined(_CFE_)
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },
#endif

  /* wlan fem_ctrl reserved signals*/
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_34_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_35_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_36_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_40_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_42_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_44_AH},

  {bp_usGpioLedReserved,       .u.us = BP_GPIO_30_AL},	// WAN RED LED
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_23_AL},

#ifdef RTAX82U
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_2_AH},	// LED Group2 Red
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_3_AH},	// LED Group2 Green
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_4_AH},	// LED Group2 Blue
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_5_AH},	// LED Group1 Blue
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_6_AH},	// LED Group1 Green
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_8_AH},	// LED Group4 Red
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_11_AH},	// LED Group3 Red
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_13_AH},	// LED Group3 Green
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_22_AH},	// LED Group1 Red
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_24_AH},	// LED Group3 Blue
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_25_AH},	// LED Group4 Blue
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_29_AH},	// LED Group4 Green
#endif

  {bp_usGpioLedWL0Act,         .u.us = BP_GPIO_31_AL | BP_LED_USE_GPIO},  /* WLAN_2G_ACT, must use BP_LED_USE_GPIO */
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_84_AL},	// USB power
  {bp_usButtonIdx,             .u.us = 2},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_7_AL},	// 4T4R
  {bp_usButtonIdx,             .u.us = 3},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_9_AL},	// ePA
#ifdef RTAX82U
  {bp_usButtonIdx,             .u.us = 4},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_10_AL},	// LED button
#endif


  {bp_usGphyBaseAddress,       .u.us = BCM963178_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = (BCM963178_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963178_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963178_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963178_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId4,                .u.ul = (BCM963178_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
#if defined(RTAX58U) || defined(TUFAX3000)
  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 0},
  {bp_usNetLed3,               .u.us = BP_GPIO_24_AL},	// P0
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 1},
  {bp_usNetLed3,               .u.us = BP_GPIO_25_AL},	// P1
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 2},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 2},
  {bp_usNetLed3,               .u.us = BP_GPIO_29_AL},	// P2
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 3},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 3},
  {bp_usNetLed3,               .u.us = BP_GPIO_10_AL},	// P3
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
#endif
  {bp_usIntfId,                .u.us = 4},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 4},
  {bp_usNetLed3,               .u.us = BP_GPIO_21_AL},	// WAN
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},

  {bp_usUsbPwrOn1,             .u.us = BP_GPIO_86_AL},
  {bp_usUsbPwrFlt1,            .u.us = BP_GPIO_85_AL},
  {bp_usUsbPwrOn0,             .u.us = BP_GPIO_84_AL},
  {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_83_AL},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_63178_DEFAULT},
  {bp_last}
};
#else
static bp_elem_t g_bcm96750ref4[] = {
  {bp_cpBoardId,               .u.cp = "96750REF4"},
  {bp_usButtonIdx,             .u.us = 0},
  {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_0_AL},
#if defined(_CFE_)
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {bp_ulButtonActionParm,      .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},
#endif
  {bp_usButtonIdx,             .u.us = 1},
  {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_1|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_1_AL},
#if defined(_CFE_)
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },
#endif

  /* wlan fem_ctrl reserved signals*/
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_34_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_35_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_36_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_40_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_42_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_44_AH},

#ifdef RTAX82_XD6
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_29_AL},	// BT RESET
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_33_AL},	// BT DISABLE
#else
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_30_AL},	// WAN RED LED
#ifdef RTAX82U
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_31_AL},	// fake 5G LED
#endif
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_23_AL},
#endif

#ifdef RTAX82U
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_2_AH},	// LED Group2 Red
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_3_AH},	// LED Group2 Green
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_4_AH},	// LED Group2 Blue
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_5_AH},	// LED Group1 Blue
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_6_AH},	// LED Group1 Green
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_8_AH},	// LED Group4 Red
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_11_AH},	// LED Group3 Red
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_13_AH},	// LED Group3 Green
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_22_AH},	// LED Group1 Red
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_24_AH},	// LED Group3 Blue
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_25_AH},	// LED Group4 Blue
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_29_AH},	// LED Group4 Green
#elif defined(RTAX82_XD6)
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_2_AL},   // LED Group Red
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_3_AL},   // LED Group Green
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_4_AL},   // LED Group Blue
#endif

#ifdef RTAX82U
  {bp_usGpioLedWL0Act,         .u.us = BP_GPIO_21_AL | BP_LED_USE_GPIO},  /* fake WLAN_2G_ACT, must use BP_LED_USE_GPIO */
#else
  {bp_usGpioLedWL0Act,         .u.us = BP_GPIO_31_AL | BP_LED_USE_GPIO},  /* WLAN_2G_ACT, must use BP_LED_USE_GPIO */
#endif
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_84_AL},	// USB power
  {bp_usButtonIdx,             .u.us = 2},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_7_AL},	// 4T4R
  {bp_usButtonIdx,             .u.us = 3},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_9_AL},	// ePA
#ifdef RTAX82U
  {bp_usButtonIdx,             .u.us = 4},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_10_AL},	// LED button
#endif


  {bp_usGphyBaseAddress,       .u.us = BCM963178_PHY_BASE},  // use phy addressses on SF2 with base address 0x8 
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH}, 
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch 
  {bp_ulPortMap,               .u.ul = 0x1f}, 
  {bp_ulPhyId0,                .u.ul = (BCM963178_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)}, 
  {bp_ulPhyId1,                .u.ul = (BCM963178_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)}, 
  {bp_ulPhyId2,                .u.ul = (BCM963178_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)}, 
  {bp_ulPhyId3,                .u.ul = (BCM963178_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)}, 
  {bp_ulPhyId4,                .u.ul = (BCM963178_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)}, 
#if defined(RTAX58U) || defined(TUFAX3000)
  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 0},
  {bp_usNetLed3,               .u.us = BP_GPIO_24_AL},	// P0
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 1},
  {bp_usNetLed3,               .u.us = BP_GPIO_25_AL},	// P1
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 2},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 2},
  {bp_usNetLed3,               .u.us = BP_GPIO_29_AL},	// P2
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 3},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 3},
  {bp_usNetLed3,               .u.us = BP_GPIO_10_AL},	// P3
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd}, 
#endif
  {bp_usIntfId,                .u.us = 4},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 4},
#if !defined(RTAX82U) && !defined(RTAX82_XD6)
  {bp_usNetLed3,               .u.us = BP_GPIO_21_AL},	// WAN
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
#endif
  {bp_usIntfEnd},

  {bp_usUsbPwrOn1,             .u.us = BP_GPIO_86_AL},
  {bp_usUsbPwrFlt1,            .u.us = BP_GPIO_85_AL},
  {bp_usUsbPwrOn0,             .u.us = BP_GPIO_84_AL},
  {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_83_AL},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_63178_DEFAULT},
  {bp_last}
};
#endif

static bp_elem_t g_bcm96750ref1[] = {
  {bp_cpBoardId,               .u.cp = "96750REF1"},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND},
  {bp_usSerialLedData,         .u.us = BP_GPIO_26_AH},
  {bp_usSerialLedClk,          .u.us = BP_GPIO_27_AH},
  {bp_usSerialLedMask,         .u.us = BP_GPIO_28_AH},
  {bp_usButtonIdx,             .u.us = 0},
  {  bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {  bp_usGpio_Intr,           .u.us = BP_GPIO_0_AL},
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {      bp_ulButtonActionParm,  .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},  
  {bp_usButtonIdx,             .u.us = 1},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_1|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {  bp_usGpio_Intr,           .u.us = BP_GPIO_1_AL},
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },

  /* wlan bluetooth co-existance reserved pins */
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_30_AL},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_31_AL},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_32_AL},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_33_AL},

  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_28_AH}, /*wlan0 led? */
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_26_AH},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_27_AH},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_29_AH},
#if !defined(_CFE_)
  {bp_usVregSync,              .u.us = BP_GPIO_2_AH},
#endif
  {bp_usGpioBitbangI2cSda,     .u.us = BP_GPIO_3_AH},
  {bp_usGpioBitbangI2cScl,     .u.us = BP_GPIO_4_AH},
  {bp_usI2sSclk,               .u.us = BP_GPIO_6_AH},
  {bp_usI2sLrck,               .u.us = BP_GPIO_7_AH},
  {bp_usI2sRxSdata,            .u.us = BP_GPIO_8_AH},
  {bp_usI2sTxSdata,            .u.us = BP_GPIO_9_AH},
  {bp_usI2sMclk,               .u.us = BP_GPIO_10_AH},
  {bp_usPcmSdin,               .u.us = BP_GPIO_13_AH},
  {bp_usPcmSdout,              .u.us = BP_GPIO_14_AH},
  {bp_usPcmClk,                .u.us = BP_GPIO_15_AH},
  {bp_usPcmFs,                 .u.us = BP_GPIO_16_AH},
  {bp_usSpiSlaveSelectNum,     .u.us = 1}, /* define the SPI select for voice */
  {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_80_AL},

  {bp_usGphyBaseAddress,       .u.us = BCM963178_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = (BCM963178_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963178_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963178_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963178_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId4,                .u.ul = (BCM963178_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},

  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 0},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_0_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_15_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 1},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_2_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_16_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 2},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 2},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_4_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_17_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 3},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 3},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_6_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_18_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 4},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 4},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_8_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_19_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},

  {bp_usIntfId,                .u.us = 6}, /* HS_UART for BT */
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_UART},
  {bp_usPortNum,               .u.us = 1},
  {bp_usUartSdin,              .u.us = BP_GPIO_21_AH},
  {bp_usUartSdout,             .u.us = BP_GPIO_20_AH},
  {bp_usUartCts,               .u.us = BP_GPIO_22_AH},
  {bp_usUartRts,               .u.us = BP_GPIO_12_AH},
  {bp_usIntfEnd},
  {bp_usGpioBtWake,            .u.us = BP_GPIO_24_AH},
  {bp_usGpioBtReset,           .u.us = BP_GPIO_29_AH},
  {bp_usUsbPwrOn1,             .u.us = BP_GPIO_86_AL},
  {bp_usUsbPwrFlt1,            .u.us = BP_GPIO_85_AL},
  {bp_usUsbPwrOn0,             .u.us = BP_GPIO_84_AL},
  {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_83_AL},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_1024MB| BP_DDR_DEVICE_WIDTH_8 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_63178_DEFAULT},
  {bp_last}
};

static bp_elem_t g_bcm96752ref1[] = {
  {bp_cpBoardId,               .u.cp = "96752REF1"},
  {bp_usButtonIdx,             .u.us = 0},
  {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_4_AL},
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {bp_ulButtonActionParm,      .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},
  {bp_usButtonIdx,             .u.us = 1},
  {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_1|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_5_AL},
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },
#if !defined(_CFE_)
  {bp_usVregSync,              .u.us = BP_GPIO_2_AH},
#endif
  {bp_usGphyBaseAddress,       .u.us = BCM963178_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x03},
  {bp_ulPhyId0,                .u.ul = (BCM963178_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963178_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},

  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 0},
  {bp_usNetLed0,               .u.us = BP_GPIO_0_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_GPIO_22_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 1},
  {bp_usNetLed0,               .u.us = BP_GPIO_7_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_GPIO_6_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 2}, /* HS_UART for BT */
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_UART},
  {bp_usPortNum,               .u.us = 1},
  {bp_usUartSdin,              .u.us = BP_GPIO_21_AH},
  {bp_usUartSdout,             .u.us = BP_GPIO_20_AH},
  {bp_usUartRts,               .u.us = BP_GPIO_12_AH},
  {bp_usUartCts,               .u.us = BP_GPIO_22_AH},
  {bp_usIntfEnd},

  /* wlan fem_ctrl reserved signals*/
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_34_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_35_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_36_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_37_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_38_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_39_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_40_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_41_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_42_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_43_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_44_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_45_AH},

  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_27_AL},


  {bp_usI2sSclk,               .u.us = BP_GPIO_29_AH},
  {bp_usI2sLrck,               .u.us = BP_GPIO_30_AH},
  {bp_usI2sRxSdata,            .u.us = BP_GPIO_31_AH},
  {bp_usI2sTxSdata,            .u.us = BP_GPIO_32_AH},
  {bp_usI2sMclk,               .u.us = BP_GPIO_33_AH},
  {bp_usPcmSdin,               .u.us = BP_GPIO_13_AH},
  {bp_usPcmSdout,              .u.us = BP_GPIO_14_AH},
  {bp_usPcmClk,                .u.us = BP_GPIO_15_AH},
  {bp_usPcmFs,                 .u.us = BP_GPIO_16_AH},
  {bp_usSpiSlaveSelectNum,     .u.us = 1}, /* define the SPI select for voice */
  {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_80_AL},
  {bp_usUsbPwrOn1,             .u.us = BP_GPIO_86_AL},
  {bp_usUsbPwrFlt1,            .u.us = BP_GPIO_85_AL},
  {bp_usUsbPwrOn0,             .u.us = BP_GPIO_84_AL},
  {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_83_AL},

  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_63178_DEFAULT},
  {bp_last}
};

static bp_elem_t g_dsl_ax82u[] = {
  {bp_cpBoardId,               .u.cp = "DSL-AX82U"},
  {bp_usButtonIdx,             .u.us = 0},
  {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_0_AL},
#if defined(_CFE_)
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {bp_ulButtonActionParm,      .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},
#endif
  {bp_usButtonIdx,             .u.us = 1},
  {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_1|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_1_AL},
#if defined(_CFE_)
  {bp_usButtonAction,          .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },
#endif

  /* wlan fem_ctrl reserved signals*/
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_35_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_38_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_41_AH},
  {bp_usGpioWlanReserved,      .u.us = BP_GPIO_44_AH},

  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_24_AL},
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_6_AL},	// PWR RED LED

  {bp_usGphyBaseAddress,       .u.us = BCM963178_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = (BCM963178_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963178_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963178_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963178_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId4,                .u.ul = (BCM963178_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},

  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 0},
  //{bp_usNetLed0,               .u.us = BP_GPIO_20_AL},	// LAN
  //{bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 1},
  //{bp_usNetLed0,               .u.us = BP_GPIO_20_AL},	// LAN
  //{bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 2},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 2},
  //{bp_usNetLed0,               .u.us = BP_GPIO_20_AL},	// LAN
  //{bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 3},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 3},
  //{bp_usNetLed0,               .u.us = BP_GPIO_20_AL},	// LAN
  //{bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 4},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 4},
  //{bp_usNetLed0,               .u.us = BP_GPIO_8_AL},	// WAN
  //{bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_ALL},
  {bp_usIntfEnd},

  {bp_usGpioLedReserved,       .u.us = BP_GPIO_20_AL},	// LAN LED
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_8_AL},	// WAN LED
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_9_AL},	// WAN RED LED

  {bp_usIntfId,                .u.us = 5},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  //{bp_usGpioLedWanLink,        .u.us = BP_GPIO_8_AL},
  //{bp_usGpioLedWanErr,         .u.us = BP_GPIO_9_AL},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_70},
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_17_AH}, // Line Driver 0 = "Int"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_18_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_19_AH},
  {bp_usIntfEnd},

  {bp_usGpioLedReserved,       .u.us = BP_GPIO_2_AH},	// LED Group3 Red
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_3_AH},	// LED Group3 Green
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_4_AH},	// LED Group3 Blue
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_5_AH},	// LED Group4 Red
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_7_AH},	// LED Group4 Blue
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_10_AH},	// LED Group4 Green
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_11_AH},	// LED Group1 Blue
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_14_AH},	// LED Group2 Red
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_15_AH},	// LED Group2 Green
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_16_AH},	// LED Group2 Blue
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_26_AH},	// LED Group1 Red
  {bp_usGpioLedReserved,       .u.us = BP_GPIO_28_AH},	// LED Group1 Green

  {bp_usGpioLedReserved,       .u.us = BP_GPIO_27_AL | BP_LED_USE_GPIO}, /* wlan0 led */
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_24_AL},

  {bp_usUsbPwrOn0,             .u.us = BP_GPIO_84_AH},
  {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_83_AL},

  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_63178_DEFAULT},
  //{bp_elemTemplate,            .u.bp_elemp = g_bcm963178ref2_base},
  {bp_last}
};

bp_elem_t * g_BoardParms[] = {g_bcm963178sv, g_bcm963178sv_p200, g_bcm963178ref1, g_bcm963178rfdvt, g_bcm963178ref3, g_bcm96750ref4, g_bcm96752ref1, g_bcm963178ref2, g_bcm963178ref1_epa, g_bcm963178ref2_p200, g_bcm963178ref2_p300, g_bcm96750ref1, g_bcm963178ref5, g_bcm963178ref6, g_dsl_ax82u, 0};


