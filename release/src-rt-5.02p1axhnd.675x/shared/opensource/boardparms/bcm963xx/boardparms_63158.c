#include "bp_defs.h"
#include "boardparms.h"
#include "bcmSpiRes.h"
extern BpCmdElem moca6802InitSeq[];


#define BCM963158_PHY_BASE             0x8

static bp_elem_t g_bcm963158sv[] = {
  {bp_cpBoardId,               .u.cp = "963158SV"},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND}, 
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_EMMC}, 
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_I2S}, 
  {bp_usExtIntrSDCardDetect,   .u.us = BP_EXT_INTR_0 | BP_EXT_INTR_TYPE_IRQ_BOTH_EDGE | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_45_AL},
  {bp_usSerialLedData,         .u.us = BP_GPIO_0_AH},
  {bp_usSerialLedClk,          .u.us = BP_GPIO_1_AH},
  {bp_usSerialLedMask,         .u.us = BP_GPIO_2_AH},
  {bp_usVregSync,              .u.us = BP_GPIO_19_AH},
  {bp_usGpioI2cSda,            .u.us = BP_GPIO_11_AH}, /* i2c for sgmii optical module */
  {bp_usGpioI2cScl,            .u.us = BP_GPIO_12_AH},
  {bp_usGphyBaseAddress,       .u.us = BCM963158_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x21},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ulPhyId5,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  //{bp_ulCrossbar,              .u.ul = 9},    /* Not available on SV board standard design */
  //{bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulCrossbar,              .u.ul = 10}, /* EtherWAN on SGPHY and use SGPHY LED */
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963158_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = (BCM963158_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963158_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963158_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963158_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},

  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_FE_ANNEXA | BP_AFE_LD_6303 |  BP_AFE_FE_REV_6303_REV_12_3_60 },
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_32_AH}, // Line Driver 0 = "Int"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_33_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_34_AH},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 1},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH1 | BP_AFE_FE_ANNEXA | BP_AFE_LD_6303 | BP_AFE_FE_REV_6303_REV_12_3_60 },
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_35_AH}, // Line Driver 1 = "Ext"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_36_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_37_AH},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 2},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_UART},
  {bp_usPortNum,               .u.us = 0},
  {bp_usUartSdin,              .u.us = BP_GPIO_106_AH},
  {bp_usUartSdout,             .u.us = BP_GPIO_107_AH},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 3},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_UART},
  {bp_usPortNum,               .u.us = 1},
  {bp_usUartSdin,              .u.us = BP_GPIO_25_AH},
  {bp_usUartSdout,             .u.us = BP_GPIO_24_AH},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 4}, /* HS_UART for BT */
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_UART},
  {bp_usPortNum,               .u.us = 2},
  {bp_usUartSdin,              .u.us = BP_GPIO_5_AH},
  {bp_usUartSdout,             .u.us = BP_GPIO_6_AH},
  {bp_usUartCts,               .u.us = BP_GPIO_3_AH}, 
  {bp_usUartRts,               .u.us = BP_GPIO_4_AH},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 5},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_UART},
  {bp_usPortNum,               .u.us = 3},
  {bp_usUartSdin,              .u.us = BP_GPIO_26_AH},
  {bp_usUartSdout,             .u.us = BP_GPIO_27_AH},
  {bp_usIntfEnd},

  {bp_usUsbPwrOn1,             .u.us = BP_GPIO_124_AL},
  {bp_usUsbPwrFlt1,            .u.us = BP_GPIO_123_AL},
  {bp_usUsbPwrOn0,             .u.us = BP_GPIO_122_AL},
  {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_121_AL},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_1067_14_14_14 | BP_DDR_TOTAL_SIZE_1024MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_32BIT | BP_DDR_SSC_CONFIG_1},
  {bp_ulPciFlags,              .u.ul = BP_PCI0_DUAL_LANE},
  {bp_last}
};

static bp_elem_t g_bcm963158dvt_lan[] = {
  {bp_cpBoardId,               .u.cp = "963158DVT_LAN"},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND}, 
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_EMMC}, 
  {bp_usSerialLedData,         .u.us = BP_GPIO_0_AH},
  {bp_usSerialLedClk,          .u.us = BP_GPIO_1_AH},
  {bp_usSerialLedMask,         .u.us = BP_GPIO_2_AH},
  /* Any serial led that are assigned in the hardware design but not used by board parameter 
     should reserve one entry below to make sure other serial led output in the right slot*/
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_13_AH},
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_14_AH},
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_15_AH},
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_16_AH},
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_17_AH},
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_26_AH},
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_27_AH},
  {bp_usButtonIdx,             .u.us = 0},
  {  bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {  bp_usGpio_Intr,           .u.us = BP_GPIO_41_AL},
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {      bp_ulButtonActionParm,  .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},  
  {bp_usButtonIdx,             .u.us = 1},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_1|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {  bp_usGpio_Intr,           .u.us = BP_GPIO_46_AL},
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_17_AL | BP_LED_USE_GPIO}, 
  {bp_usSpiSlaveSelectNum,     .u.us = 1},
  {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_112_AL},
  {bp_usSpiSlaveSelectNum,     .u.us = 3},
  {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_9_AL},
  {bp_usGphyBaseAddress,       .u.us = BCM963158_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x21},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ulPhyId5,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_WAN_ONLY }, 
  {bp_ulCrossbar,              .u.ul = 10}, /* EtherWAN on SGPHY and use SGPHY LED */
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963158_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x5f},
  {bp_ulPhyId0,                .u.ul = (BCM963158_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963158_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963158_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963158_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  /* uncomment below line to use RGMII2 with external PHY on p3 */
  /*{bp_ulPhyId3,                .u.ul = 0x0 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},*/
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 12}, /* RGMII0 */
  {bp_ulCrossbarPhyId,         .u.ul = 0x18 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},
  {bp_ulPhyId6,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 11}, /* RGMII1 */
  {bp_ulCrossbarPhyId,         .u.ul = 0x19 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulCrossbarPhyId,         .u.ul = 0x1e | PHY_EXTERNAL | PHY_TYPE_CL45GPHY},
  {bp_usGpioLedAggregateAct,   .u.us = BP_SERIAL_GPIO_28_AH},
  {bp_usGpioLedAggregateLnk,   .u.us = BP_SERIAL_GPIO_29_AH},

  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_18_AL},
  {bp_usGpioLedWanAct,         .u.us = BP_SERIAL_GPIO_30_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6304 | BP_AFE_FE_REV_6304_REV_12_4_60 },
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_32_AH}, // Line Driver 0 = "Int"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_33_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_34_AH},
  {bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_10_AH},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 1},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_19_AL},
  {bp_usGpioLedWanAct,         .u.us = BP_SERIAL_GPIO_31_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH1 | BP_AFE_LD_6304 |  BP_AFE_FE_REV_6304_REV_12_4_60 },
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_35_AH}, // Line Driver 1 = "Ext"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_36_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_37_AH},
  {bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_10_AH},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 2},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 0},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_0_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_1_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_20_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 3},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 1},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_3_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_21_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 4},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 2},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_5_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_22_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 5},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 3},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_6_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_7_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_23_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 6},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = SF2_WAN_PORT_NUM},
  {bp_ulCrossbar,              .u.ul = 10},  /* SGPHY on EtherWAN port */
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_9_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_24_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 7},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
  {bp_usPortNum,               .u.us = SF2_WAN_PORT_NUM},
  {bp_ulCrossbar,              .u.ul = 9},  /* SGMII on EtherWAN port */
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_10_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_11_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed2,               .u.us = BP_SERIAL_GPIO_12_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_25_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},

  {bp_usUsbPwrOn1,             .u.us = BP_GPIO_124_AL},
  {bp_usUsbPwrFlt1,            .u.us = BP_GPIO_123_AL},
  {bp_usUsbPwrOn0,             .u.us = BP_GPIO_122_AL},
  {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_121_AL},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_GPIO_15_AL | BP_LED_USE_GPIO},
  {bp_usGpioVoip2Led,          .u.us = BP_GPIO_16_AL | BP_LED_USE_GPIO},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_1067_14_14_14 | BP_DDR_TOTAL_SIZE_1024MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_32BIT | BP_DDR_SSC_CONFIG_1},
  {bp_ulPciFlags,              .u.ul = BP_PCI0_DUAL_LANE},
  {bp_last}
};

static bp_elem_t g_bcm963158dvt[] = {
  {bp_cpBoardId,               .u.cp = "963158DVT"},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND}, 
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_EMMC}, 
  {bp_usSerialLedData,         .u.us = BP_GPIO_0_AH},
  {bp_usSerialLedClk,          .u.us = BP_GPIO_1_AH},
  {bp_usSerialLedMask,         .u.us = BP_GPIO_2_AH},
  /* Any serial led that are assigned in the hardware design but not used by board parameter 
     should reserve one entry below to make sure other serial led output in the right slot*/
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_16_AH},
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_17_AH},
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_27_AH},
  {bp_usButtonIdx,             .u.us = 0},
  {  bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {  bp_usGpio_Intr,           .u.us = BP_GPIO_41_AL},
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {      bp_ulButtonActionParm,  .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},  
  {bp_usButtonIdx,             .u.us = 1},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_1|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {  bp_usGpio_Intr,           .u.us = BP_GPIO_46_AL},
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_17_AL | BP_LED_USE_GPIO}, 
  {bp_usSpiSlaveSelectNum,     .u.us = 1},
  {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_112_AL},
  {bp_usSpiSlaveSelectNum,     .u.us = 3},
  {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_9_AL},
  {bp_usGphyBaseAddress,       .u.us = BCM963158_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x21},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ulPhyId5,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_WAN_ONLY }, 
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulCrossbarPhyId,         .u.ul = 0x1e | PHY_EXTERNAL | PHY_TYPE_CL45GPHY},
  {bp_ulCrossbar,              .u.ul = 10}, /* EtherWAN on SGPHY and use SGPHY LED */
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963158_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x5f},
  {bp_ulPhyId0,                .u.ul = (BCM963158_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963158_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963158_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963158_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  /* uncomment below line to use RGMII2 with external PHY on p3 */
  /*{bp_ulPhyId3,                .u.ul = 0x0 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},*/
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 12}, /* RGMII0 */
  {bp_ulCrossbarPhyId,         .u.ul = 0x18 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},
  {bp_ulPhyId6,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 11}, /* RGMII1 */
  {bp_ulCrossbarPhyId,         .u.ul = 0x19 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},
  {bp_usGpioLedAggregateAct,   .u.us = BP_SERIAL_GPIO_28_AH},
  {bp_usGpioLedAggregateLnk,   .u.us = BP_SERIAL_GPIO_29_AH},

  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_18_AL},
  {bp_usGpioLedWanAct,         .u.us = BP_SERIAL_GPIO_30_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6304 | BP_AFE_FE_REV_6304_REV_12_4_60 },
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_32_AH}, // Line Driver 0 = "Int"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_33_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_34_AH},
  {bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_10_AH},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 1},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_19_AL},
  {bp_usGpioLedWanAct,         .u.us = BP_SERIAL_GPIO_31_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH1 | BP_AFE_LD_6304 |  BP_AFE_FE_REV_6304_REV_12_4_60 },
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_35_AH}, // Line Driver 1 = "Ext"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_36_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_37_AH},
  {bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_10_AH},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 2},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 0},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_0_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_1_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_20_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 3},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 1},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_3_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_21_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 4},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 2},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_5_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_22_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 5},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 3},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_6_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_7_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_23_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 6},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = SF2_WAN_PORT_NUM},
  {bp_ulCrossbar,              .u.ul = 10},  /* SGPHY on EtherWAN port */
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_9_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_24_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 7},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
  {bp_usPortNum,               .u.us = SF2_WAN_PORT_NUM},
  {bp_ulCrossbar,              .u.ul = 9},  /* SGMII on EtherWAN port */
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_10_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_11_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed2,               .u.us = BP_SERIAL_GPIO_12_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_25_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 8},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xPON},     /* in AE mode */
  {bp_usPortNum,               .u.us = 0},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_13_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_14_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed2,               .u.us = BP_SERIAL_GPIO_15_AL},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500|BP_NET_LED_SPEED_10G},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_26_AL},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},

  {bp_usUsbPwrOn1,             .u.us = BP_GPIO_124_AL},
  {bp_usUsbPwrFlt1,            .u.us = BP_GPIO_123_AL},
  {bp_usUsbPwrOn0,             .u.us = BP_GPIO_122_AL},
  {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_121_AL},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_GPIO_15_AL | BP_LED_USE_GPIO},
  {bp_usGpioVoip2Led,          .u.us = BP_GPIO_16_AL | BP_LED_USE_GPIO},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_1067_14_14_14 | BP_DDR_TOTAL_SIZE_1024MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_32BIT | BP_DDR_SSC_CONFIG_1},
  {bp_ulPciFlags,              .u.ul = BP_PCI0_DUAL_LANE},
  {bp_last}
};

static bp_elem_t g_bcm963158dvt_xgae[] = {
  {bp_cpBoardId,               .u.cp = "963158DVT_XGAE"},
  {bp_usGphyBaseAddress,       .u.us = BCM963158_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x11},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ulPhyId4,                .u.ul = 0|MAC_IF_XGAE_SERDES},  /* Dummy PHY ID */
  {bp_ulPhyId4,                .u.ul = 0x1f | PHY_EXTERNAL | PHY_TYPE_CL45GPHY},
  {bp_ulPhyId5,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_WAN_ONLY }, 
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x5f},
  {bp_ulPhyId0,                .u.ul = (BCM963158_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963158_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963158_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963158_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  /* uncomment below line to use RGMII2 with external PHY on p3 */
  /*{bp_ulPhyId3,                .u.ul = 0x0 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},*/
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 12}, /* RGMII0 */
  {bp_ulCrossbarPhyId,         .u.ul = 0x18 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963158_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId6,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 11}, /* RGMII1 */
  {bp_ulCrossbarPhyId,         .u.ul = 0x19 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulCrossbarPhyId,         .u.ul = 0x1e | PHY_EXTERNAL | PHY_TYPE_CL45GPHY},
  {bp_usGpioLedAggregateAct,   .u.us = BP_SERIAL_GPIO_28_AH},
  {bp_usGpioLedAggregateLnk,   .u.us = BP_SERIAL_GPIO_29_AH},

  {bp_elemTemplate,            .u.bp_elemp = g_bcm963158dvt},
  {bp_last}
};

static bp_elem_t g_bcm963158dvt_p200[] = {
  {bp_cpBoardId,               .u.cp = "963158DVT_P200"},

  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_18_AH},
  {bp_usGpioLedWanAct,         .u.us = BP_SERIAL_GPIO_30_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6305 | BP_AFE_FE_REV_6305_REV_12_5_60_2|BP_AFE_FE_RNC},
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_32_AH}, // Line Driver 0 = "Int"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_33_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_34_AH},
  {bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_11_AH},
  {bp_usGpioAFELDRelay,        .u.us = BP_GPIO_38_AH},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 1},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_19_AH},
  {bp_usGpioLedWanAct,         .u.us = BP_SERIAL_GPIO_31_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH1 | BP_AFE_LD_6305 | BP_AFE_FE_REV_6305_REV_12_5_60_1|BP_AFE_FE_RNC},
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_35_AH}, // Line Driver 1 = "Ext"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_36_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_37_AH},
  {bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_18_AH},
  {bp_usGpioAFELDRelay,        .u.us = BP_GPIO_3_AH},
  {bp_usIntfEnd},

//secondary definitions for AfeIds
  {bp_usIntfId,                .u.us = 9},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_60},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 10},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 1},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH1 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_60},
  {bp_usIntfEnd},


  {bp_elemTemplate,            .u.bp_elemp = g_bcm963158dvt},
  {bp_last}
};

static bp_elem_t g_bcm963158dvt_p200_xgae[] = {
  {bp_cpBoardId,               .u.cp = "963158DVT_P200X"},
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x11},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ulPhyId4,                .u.ul = 0|MAC_IF_XGAE_SERDES},  /* Dummy PHY ID */
  {bp_ulPhyId4,                .u.ul = 0x1f | PHY_EXTERNAL | PHY_TYPE_CL45GPHY},
  {bp_ulPhyId5,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_WAN_ONLY }, 
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x5f},
  {bp_ulPhyId0,                .u.ul = (BCM963158_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963158_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963158_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963158_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  /* uncomment below line to use RGMII2 with external PHY on p3 */
  /*{bp_ulPhyId3,                .u.ul = 0x0 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},*/
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 12}, /* RGMII0 */
  {bp_ulCrossbarPhyId,         .u.ul = 0x18 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963158_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId6,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 11}, /* RGMII1 */
  {bp_ulCrossbarPhyId,         .u.ul = 0x19 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulCrossbarPhyId,         .u.ul = 0x1e | PHY_EXTERNAL | PHY_TYPE_CL45GPHY},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963158dvt_p200},
  {bp_last}
};

static bp_elem_t g_bcm963158ref1[] = {
  {bp_cpBoardId,               .u.cp = "963158REF1"},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND}, 
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_EMMC}, 
  {bp_usSerialLedData,         .u.us = BP_GPIO_0_AH},
  {bp_usSerialLedClk,          .u.us = BP_GPIO_1_AH},
  {bp_usSerialLedMask,         .u.us = BP_GPIO_2_AH},
  /* Any serial led that are assigned in the hardware design but not used by board parameter 
     should reserve one entry below to make sure other serial led output in the right slot*/
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_16_AH},
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_17_AH},
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_27_AH},

  {bp_usButtonIdx,             .u.us = 0},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {  bp_usGpio_Intr,           .u.us = BP_GPIO_46_AL},
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {      bp_ulButtonActionParm, .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},  
  {bp_usButtonIdx,             .u.us = 1},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_1|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {  bp_usGpio_Intr,           .u.us = BP_GPIO_41_AL},
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },

  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_29_AH}, 
  {bp_usSpiSlaveSelectNum,     .u.us = 1},
  {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_112_AL},
  {bp_usSpiSlaveSelectNum,     .u.us = 4},
  {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_8_AL},  
  {bp_usGphyBaseAddress,       .u.us = BCM963158_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
#if defined(CONFIG_BCM_SYSPORT)
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
#else
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x21},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ulPhyId5,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963158_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
#endif
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = (BCM963158_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963158_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963158_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963158_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
#if 0 /* for RGMII daughter card */
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 12},
  {bp_ulCrossbarPhyId,         .u.ul = 0x19 | PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},  /* make sure the phy id matches the one on the plug in rgmii phy daughter card */
#endif

  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_GPIO_18_AH},
  {bp_usGpioLedWanAct,         .u.us = BP_GPIO_30_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6304 | BP_AFE_FE_REV_6304_REV_12_4_60 },
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_32_AH}, // Line Driver 0 = "Int"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_33_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_34_AH},
  {bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_38_AH},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 1},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_19_AH},
  {bp_usGpioLedWanAct,         .u.us = BP_GPIO_31_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH1 | BP_AFE_LD_6304 |  BP_AFE_FE_REV_6304_REV_12_4_60 },
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_35_AH}, // Line Driver 1 = "Ext"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_36_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_37_AH},
  {bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_38_AH},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 2},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 0},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_0_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_1_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_20_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 3},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 1},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_2_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_3_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_21_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 4},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 2},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_4_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_5_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_22_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 5},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 3},
  {bp_usNetLed0,               .u.us = BP_GPIO_6_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_GPIO_7_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_GPIO_23_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 6},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = SF2_WAN_PORT_NUM}, /* SGPHY on EtherWAN port */
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_8_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_9_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_24_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 7},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
  {bp_usPortNum,               .u.us = SF2_WAN_PORT_NUM}, /* SGMII on EtherWAN port */
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_10_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_11_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed2,               .u.us = BP_SERIAL_GPIO_12_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_25_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
  {bp_usIntfMgmtBusNum,        .u.us = 1},
  {bp_usExtIntrOpticalModulePresence, .u.us = BP_EXT_INTR_2 | BP_EXT_INTR_TYPE_IRQ_BOTH_EDGE | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_20_AL},
  {bp_usGpioSfpModDetect,      .u.us = BP_GPIO_20_AL},
  /* serdes sgmii detection pin conflict with VREG_SYNC in A0. It will be moved to another 
     pin in B0 */
  {bp_usSfpSigDetect,         .u.us = BP_GPIO_19_AH },
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 8},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xPON},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_26_AH},  /* For GPON ranging software led */
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_13_AH},  /* For AE hardware led */
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_14_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed2,               .u.us = BP_SERIAL_GPIO_15_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500|BP_NET_LED_SPEED_10G},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_SPEED_2500|BP_NET_LED_SPEED_10G},
  {bp_usExtIntrOpticalModulePresence, .u.us = BP_EXT_INTR_3 | BP_EXT_INTR_TYPE_IRQ_BOTH_EDGE | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_9_AL},
  {bp_usGpioSfpModDetect,      .u.us = BP_GPIO_9_AL},
  {bp_usGpioPonTxEn,           .u.us = BP_GPIO_3_AH},
  {bp_usRogueOnuEn,            .u.us = BP_GPIO_40_AH},
  {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
  {bp_usIntfMgmtBusNum,        .u.us = 0},
  {bp_usSfpSigDetect,          .u.us = BP_GPIO_10_AH },
  {bp_usTsync1pps,             .u.us = BP_GPIO_13_AH },
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 9},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioI2cSda,            .u.us = BP_GPIO_24_AH }, /* i2c for pon optical module */
  {bp_usGpioI2cScl,            .u.us = BP_GPIO_25_AH },
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 10},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
  {bp_usPortNum,               .u.us = 1},
  {bp_usGpioI2cSda,            .u.us = BP_GPIO_15_AH }, /* i2c for sgmii optical module */
  {bp_usGpioI2cScl,            .u.us = BP_GPIO_16_AH },
  {bp_usIntfEnd},
  {bp_usUsbPwrOn1,             .u.us = BP_GPIO_124_AL},
  {bp_usUsbPwrFlt1,            .u.us = BP_GPIO_123_AL},
  {bp_usUsbPwrOn0,             .u.us = BP_GPIO_122_AL},
  {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_121_AL},
  //  {bp_usVregSync,              .u.us = BP_GPIO_19_AH},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_GPIO_28_AL},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_1067_14_14_14 | BP_DDR_TOTAL_SIZE_1024MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_32BIT | BP_DDR_SSC_CONFIG_1},
  {bp_ulPciFlags,              .u.ul = BP_PCI0_DUAL_LANE},
  {bp_last}
};

static bp_elem_t g_bcm963158ref1_10gae[] = {
  {bp_cpBoardId,               .u.cp = "963158REF1_XGAE"},
  {bp_usGphyBaseAddress,       .u.us = BCM963158_PHY_BASE},  // use phy addresses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x11},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ulPhyId4,                .u.ul = 0|MAC_IF_XGAE_SERDES},  /* Dummy PHY ID */
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x5f},
  {bp_ulPhyId0,                .u.ul = (BCM963158_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963158_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963158_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963158_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963158_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId6,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},

  {bp_usIntfId,                .u.us = 6},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 4}, /* SGPHY on switch port 4*/
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_8_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_9_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_24_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},

  {bp_usIntfId,                .u.us = 7},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
  {bp_usPortNum,               .u.us = 6}, /* SGMII on switch port 6*/
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_10_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_11_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed2,               .u.us = BP_SERIAL_GPIO_12_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_25_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
  {bp_usIntfMgmtBusNum,        .u.us = 1},
  {bp_usExtIntrOpticalModulePresence, .u.us = BP_EXT_INTR_2 | BP_EXT_INTR_TYPE_IRQ_BOTH_EDGE | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_20_AL},
  {bp_usGpioSfpModDetect,      .u.us = BP_GPIO_20_AL},
  /* serdes sgmii detection pin conflict with VREG_SYNC in A0. It will be moved to another 
     pin in B0 */
  {bp_usSfpSigDetect,         .u.us = BP_GPIO_19_AH },
  {bp_usIntfEnd},

  {bp_elemTemplate,            .u.bp_elemp = g_bcm963158ref1},
  {bp_last}
};

static bp_elem_t g_bcm963158ref1_2p5w[] = {
  {bp_cpBoardId,               .u.cp = "963158REF1_2P5W"},
  {bp_usGphyBaseAddress,       .u.us = BCM963158_PHY_BASE},  // use phy addresses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x21},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ulPhyId5,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963158_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = (BCM963158_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963158_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963158_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963158_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},

  {bp_usIntfId,                .u.us = 6},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 4}, /* SGPHY on switch port 4*/
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_8_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_9_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_24_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},

  {bp_usIntfId,                .u.us = 7},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
  {bp_usPortNum,               .u.us = 6}, /* SGMII on switch port 6*/
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_10_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_11_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed2,               .u.us = BP_SERIAL_GPIO_12_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_25_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
  {bp_usIntfMgmtBusNum,        .u.us = 1},
  {bp_usExtIntrOpticalModulePresence, .u.us = BP_EXT_INTR_2 | BP_EXT_INTR_TYPE_IRQ_BOTH_EDGE | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_20_AL},
  {bp_usGpioSfpModDetect,      .u.us = BP_GPIO_20_AL},
  /* serdes sgmii detection pin conflict with VREG_SYNC in A0. It will be moved to another 
     pin in B0 */
  {bp_usSfpSigDetect,         .u.us = BP_GPIO_19_AH },
  {bp_usIntfEnd},

  {bp_elemTemplate,            .u.bp_elemp = g_bcm963158ref1},
  {bp_last}
};

static bp_elem_t g_bcm963158ref1_10gag[] = { // Similar to 10gae but with additional plug-in Geth card instead of serdes
  {bp_cpBoardId,               .u.cp = "963158REF1_XGAG"},
  {bp_usGphyBaseAddress,       .u.us = BCM963158_PHY_BASE},  // use phy addresses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x11},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ulPhyId4,                .u.ul = 0|MAC_IF_XGAE_SERDES},  /* Dummy PHY ID */
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x5f},
  {bp_ulPhyId0,                .u.ul = (BCM963158_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963158_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963158_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963158_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 12},
  {bp_ulCrossbarPhyId,         .u.ul = 0x19 | PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},  /* make sure the phy id matches the one on the plug in rgmii phy daughter card */
  {bp_ulPhyId6,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963158_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},

  {bp_usIntfId,                .u.us = 6},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 6}, /* SGPHY on switch port 6*/
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_8_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_9_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_24_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},

  {bp_elemTemplate,            .u.bp_elemp = g_bcm963158ref1_10gae},
  {bp_last}
};

static bp_elem_t g_bcm963158ref1_p20x []= {
  {bp_cpBoardId,               .u.cp = "963158REF1_P20X"},
  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_GPIO_18_AH},
  {bp_usGpioLedWanAct,         .u.us = BP_GPIO_30_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6304 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6304_REV_12_4_60|BP_AFE_FE_RNC },
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_32_AH}, // Line Driver 0 = "Int"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_33_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_34_AH},
  {bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_38_AH},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 1},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_19_AH},
  {bp_usGpioLedWanAct,         .u.us = BP_GPIO_31_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH1 | BP_AFE_LD_6304 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6304_REV_12_4_60|BP_AFE_FE_RNC },
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_35_AH}, // Line Driver 1 = "Ext"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_36_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_37_AH},
  {bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_38_AH},
  {bp_usIntfEnd},
//secondary definitions for AfeIds
  {bp_usIntfId,                .u.us = 11},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6304 | BP_AFE_FE_ANNEXA |  BP_AFE_FE_REV_6304_REV_12_4_60|BP_AFE_FE_RNC},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 12},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 1},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH1 | BP_AFE_LD_6304 | BP_AFE_FE_ANNEXA |  BP_AFE_FE_REV_6304_REV_12_4_60|BP_AFE_FE_RNC},
  {bp_usIntfEnd},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963158ref1},
  {bp_last}

};

static bp_elem_t g_bcm963158ref1d[] = {
  {bp_cpBoardId,               .u.cp = "963158REF1D"},
  {bp_usIntfId,                .u.us = 7},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
  {bp_usPortNum,               .u.us = SF2_WAN_PORT_NUM}, /* SGMII on EtherWAN port */
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_10_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_11_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed2,               .u.us = BP_SERIAL_GPIO_12_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_25_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
  {bp_usIntfMgmtBusNum,        .u.us = 1},
  {bp_usExtIntrOpticalModulePresence, .u.us = BP_EXT_INTR_2 | BP_EXT_INTR_TYPE_IRQ_BOTH_EDGE | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_20_AL},
  {bp_usGpioSfpModDetect,      .u.us = BP_GPIO_20_AL},
  {bp_usSfpSigDetect,          .u.us = BP_GPIO_21_AL },
  {bp_usIntfEnd},
  {bp_usGpioWireless0Disable,     .u.us = BP_GPIO_65_AH},
  {bp_usGpioWireless2Disable,     .u.us = BP_GPIO_73_AH},
  {bp_usGpioWireless3Disable,     .u.us = BP_GPIO_78_AH},
#if !defined(_CFE_)
  {bp_usVregSync,              .u.us = BP_GPIO_19_AH},
#endif
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_TYPE_DDR4 | BP_DDR_SPEED_1067_15_15_15 | BP_DDR_TOTAL_SIZE_1024MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_32BIT | BP_DDR_SSC_CONFIG_1},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963158ref1_p20x},
  {bp_last}
};


static bp_elem_t g_bcm963158ref1d_10gae[] = {
  {bp_cpBoardId,               .u.cp = "158REF1D_XGAE"},
  {bp_usGphyBaseAddress,       .u.us = BCM963158_PHY_BASE},  // use phy addresses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x11},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ulPhyId4,                .u.ul = 0|MAC_IF_XGAE_SERDES},  /* Dummy PHY ID */
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x5f},
  {bp_ulPhyId0,                .u.ul = (BCM963158_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963158_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963158_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963158_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963158_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId6,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},

  {bp_usIntfId,                .u.us = 6},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 4}, /* SGPHY on switch port 4*/
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_8_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_9_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_24_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},

  {bp_usIntfId,                .u.us = 7},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
  {bp_usPortNum,               .u.us = 6}, /* SGMII on switch port 6*/
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_10_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_11_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed2,               .u.us = BP_SERIAL_GPIO_12_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_25_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
  {bp_usIntfMgmtBusNum,        .u.us = 1},
  {bp_usExtIntrOpticalModulePresence, .u.us = BP_EXT_INTR_2 | BP_EXT_INTR_TYPE_IRQ_BOTH_EDGE | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_20_AL},
  {bp_usGpioSfpModDetect,      .u.us = BP_GPIO_20_AL},
  {bp_usSfpSigDetect,          .u.us = BP_GPIO_21_AL},
  {bp_usIntfEnd},

  {bp_elemTemplate,            .u.bp_elemp = g_bcm963158ref1d},
  {bp_last}
};

static bp_elem_t g_bcm963158ref1d_10gag[] = { // Similar to 10gae but with additional plug-in Geth card instead of serdes
  {bp_cpBoardId,               .u.cp = "158REF1D_XGAG"},
  {bp_usGphyBaseAddress,       .u.us = BCM963158_PHY_BASE},  // use phy addresses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x11},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ulPhyId4,                .u.ul = 0|MAC_IF_XGAE_SERDES},  /* Dummy PHY ID */
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x5f},
  {bp_ulPhyId0,                .u.ul = (BCM963158_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963158_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963158_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963158_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 12},
  {bp_ulCrossbarPhyId,         .u.ul = 0x19 | PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},  /* make sure the phy id matches the one on the plug in rgmii phy daughter card */
  {bp_ulPhyId6,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963158_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},

  {bp_usIntfId,                .u.us = 6},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 6}, /* SGPHY on switch port 6*/
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_8_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_9_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_24_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},

  {bp_elemTemplate,            .u.bp_elemp = g_bcm963158ref1d},
  {bp_last}
};


static bp_elem_t g_bcm963158ref2[] = {
  {bp_cpBoardId,               .u.cp = "963158REF2"},
  {bp_usSerialLedData,         .u.us = BP_GPIO_0_AH},
  {bp_usSerialLedClk,          .u.us = BP_GPIO_1_AH},
  {bp_usSerialLedMask,         .u.us = BP_GPIO_2_AH},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND}, 
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_EMMC}, 
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_13_AH},
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_14_AH},
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_15_AH},
  {bp_usButtonIdx,             .u.us = 0},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_0 },
  {  bp_usGpio_Intr,           .u.us = BP_GPIO_46_AL},
  {  bp_usButtonAction,        .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },
#if 0
  {bp_usButtonIdx,             .u.us = 1},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_1 },
  {  bp_usGpio_Intr,           .u.us = BP_GPIO_89_AL},
  {  bp_usButtonAction,        .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {    bp_ulButtonActionParm,  .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
  {  bp_usButtonAction,        .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {  bp_usButtonAction,        .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},  
#endif
  {bp_usExtIntrSDCardDetect,   .u.us = BP_EXT_INTR_2 | BP_EXT_INTR_TYPE_IRQ_BOTH_EDGE | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_90_AL},

  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_27_AH}, 
  {bp_usSpiSlaveSelectNum,     .u.us = 1},
  {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_112_AL},
  {bp_usSpiSlaveSelectNum,     .u.us = 5},
  {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_7_AL},  
  {bp_usSpiSlaveSelectNum,     .u.us = 4},
  {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_8_AL},  
  {bp_usI2sMclk,               .u.us = BP_GPIO_28_AH},
  {bp_usI2sLrck,               .u.us = BP_GPIO_29_AH},
  {bp_usI2sSclk,               .u.us = BP_GPIO_30_AH},
  {bp_usI2sSdata,              .u.us = BP_GPIO_31_AH},

  {bp_usGphyBaseAddress,       .u.us = BCM963158_PHY_BASE},  // use phy addresses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x21},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ulPhyId5,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963158_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = (BCM963158_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963158_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963158_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963158_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
#if 0 /* for RGMII daughter card */
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 12},
  {bp_ulCrossbarPhyId,         .u.ul = 0x18 | PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},  /* make sure the phy id matches the one on the plug in rgmii phy daughter card */
#endif

  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_17_AH},
  {bp_usGpioLedWanAct,         .u.us = BP_SERIAL_GPIO_30_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_FE_ANNEXA | BP_AFE_LD_6303 |  BP_AFE_FE_REV_6303_REV_12_3_60|BP_AFE_FE_RNC },
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_32_AH}, // Line Driver 0 = "Int"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_33_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_34_AH},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 1},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_16_AH},
  {bp_usGpioLedWanAct,         .u.us = BP_SERIAL_GPIO_31_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH1 | BP_AFE_FE_ANNEXA | BP_AFE_LD_6303 | BP_AFE_FE_REV_6303_REV_12_3_60|BP_AFE_FE_RNC },
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_35_AH}, // Line Driver 1 = "Ext"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_36_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_37_AH},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 2},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 0},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_0_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_1_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_20_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 3},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 1},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_2_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_3_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_21_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 4},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 2},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_4_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_5_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_22_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 5},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 3},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_6_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_7_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_23_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 6},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = SF2_WAN_PORT_NUM},
  {bp_ulCrossbar,              .u.ul = 10},  /* SGPHY on EtherWAN port */
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_8_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_9_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_24_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 7},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
  {bp_usPortNum,               .u.us = SF2_WAN_PORT_NUM},
  {bp_ulCrossbar,              .u.ul = 9},  /* SGMII on EtherWAN port */
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_10_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_11_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed2,               .u.us = BP_SERIAL_GPIO_12_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_25_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
  {bp_usIntfMgmtBusNum,        .u.us = 1},
  {bp_usExtIntrOpticalModulePresence, .u.us = BP_EXT_INTR_3 | BP_EXT_INTR_TYPE_IRQ_BOTH_EDGE | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_17_AL},
  {bp_usGpioSfpModDetect,      .u.us = BP_GPIO_17_AL},
  /* install resistor option to connect LAN_FIBER_DETECT to GPIO 19 
     can not use the VSYNC on GPIO 19 at the same time */
  {bp_usSfpSigDetect,          .u.us = BP_GPIO_19_AH },
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 8},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xPON},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_26_AH},
  {bp_InvSerdesRxPol,          .u.us = pmd_polarity_invert},
  {bp_usGpioPonTxEn,           .u.us = BP_GPIO_41_AH},
  {bp_usRogueOnuEn,            .u.us = BP_GPIO_40_AH},
  {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
  {bp_usIntfMgmtBusNum,        .u.us = 0},
  {bp_usSfpSigDetect,          .u.us = BP_GPIO_10_AH },
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 9},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioI2cSda,            .u.us = BP_GPIO_11_AH }, /* i2c for pon optical module */
  {bp_usGpioI2cScl,            .u.us = BP_GPIO_12_AH },
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 10},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
  {bp_usPortNum,               .u.us = 1},
  {bp_usGpioI2cSda,            .u.us = BP_GPIO_26_AH }, /* i2c for sgmii optical module */
  {bp_usGpioI2cScl,            .u.us = BP_GPIO_27_AH },
  {bp_usIntfEnd},
  {bp_usUsbPwrOn1,             .u.us = BP_GPIO_124_AL},
  {bp_usUsbPwrFlt1,            .u.us = BP_GPIO_123_AL},
  {bp_usUsbPwrOn0,             .u.us = BP_GPIO_122_AL},
  {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_121_AL},
  //  {bp_usVregSync,              .u.us = BP_GPIO_19_AH},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},

  {bp_usGpioLedAggregateLnk,   .u.us = BP_SERIAL_GPIO_28_AH},
  {bp_usGpioLedAggregateAct,   .u.us = BP_SERIAL_GPIO_29_AH},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_18_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_19_AL},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_1067_14_14_14 | BP_DDR_TOTAL_SIZE_2048MB| BP_DDR_DEVICE_WIDTH_8 | BP_DDR_TOTAL_WIDTH_32BIT | BP_DDR_SSC_CONFIG_1},
  {bp_last}
};

static bp_elem_t g_bcm963158ref3[] = {
  {bp_cpBoardId,               .u.cp = "963158REF3"},
  {bp_usGphyBaseAddress,       .u.us = BCM963158_PHY_BASE},  // use phy addresses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x21},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ulPhyId5,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963158_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = (BCM963158_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963158_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963158_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963158_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
#if 0 /* for RGMII daughter card */
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 12},
  {bp_ulCrossbarPhyId,         .u.ul = 0x18 | PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},  /* make sure the phy id matches the one on the plug in rgmii phy daughter card */
#endif

  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_GPIO_18_AH},
  {bp_usGpioLedWanAct,         .u.us = BP_GPIO_30_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6305 | BP_AFE_FE_REV_6305_REV_12_5_60_2},
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_32_AH}, // Line Driver 0 = "Int"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_33_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_34_AH},
  {bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_11_AH},
  {bp_usGpioAFELDRelay,        .u.us = BP_GPIO_38_AH},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 1},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_19_AH},
  {bp_usGpioLedWanAct,         .u.us = BP_GPIO_31_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH1 | BP_AFE_LD_6305 | BP_AFE_FE_REV_6305_REV_12_5_60_1},
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_35_AH}, // Line Driver 1 = "Ext"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_36_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_37_AH},
  {bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_11_AH},
  {bp_usGpioAFELDRelay,        .u.us = BP_GPIO_39_AH},
  {bp_usIntfEnd},
//secondary definitions for AfeIds
  {bp_usIntfId,                .u.us = 11},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_60},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 12},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 1},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH1 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_60},
  {bp_usIntfEnd},

  {bp_usIntfId,                .u.us = 7},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
  {bp_usPortNum,               .u.us = SF2_WAN_PORT_NUM},
  {bp_ulCrossbar,              .u.ul = 9},  /* SGMII on EtherWAN port */
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_10_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_11_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed2,               .u.us = BP_SERIAL_GPIO_12_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_25_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
  {bp_usIntfMgmtBusNum,        .u.us = 1},
  {bp_usExtIntrOpticalModulePresence, .u.us = BP_EXT_INTR_2 | BP_EXT_INTR_TYPE_IRQ_BOTH_EDGE | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_20_AL},
  {bp_usGpioSfpModDetect,      .u.us = BP_GPIO_20_AL},
  {bp_usSfpSigDetect,         .u.us = BP_GPIO_19_AH },
  {bp_usIntfEnd},

  {bp_usIntfId,                .u.us = 8},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xPON},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_26_AH},
  {bp_InvSerdesRxPol,          .u.us = pmd_polarity_invert},
  {bp_InvSerdesTxPol,          .u.us = pmd_polarity_invert},
  {bp_usRogueOnuEn,            .u.us = BP_GPIO_40_AH},
  {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
  {bp_usIntfMgmtBusNum,        .u.us = 0},
  {bp_usSfpSigDetect,          .u.us = BP_GPIO_10_AH },
//  {bp_usPmdMACEwakeEn,         .u.us = BP_GPIO_3_AH}, /* has a flaw with this pinmux */
  {bp_usExtIntrPmdAlarm,       .u.us = BP_EXT_INTR_3 | BP_EXT_INTR_TYPE_IRQ_HIGH_LEVEL | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_40_AH},
  {bp_usGpioPmdReset,          .u.us = BP_GPIO_5_AL},
  {bp_usGpioLedSim,            .u.us = BP_GPIO_NONE},
  {bp_pmdFunc,                 .u.us = BP_PMD_APD_TYPE_BOOST | BP_PMD_APD_REG_DISABLED},
  {bp_usIntfEnd},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963158ref1},
  {bp_last}
};

static bp_elem_t g_bcm963158ref3_p20x[] = {
  {bp_cpBoardId,               .u.cp = "963158REF3_P20X"},

  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_GPIO_18_AH},
  {bp_usGpioLedWanAct,         .u.us = BP_GPIO_30_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6305 | BP_AFE_FE_REV_6305_REV_12_5_60_2},
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_32_AH}, // Line Driver 0 = "Int"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_33_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_34_AH},
  {bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_11_AH},
  {bp_usGpioAFELDRelay,        .u.us = BP_GPIO_38_AH},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 1},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_19_AH},
  {bp_usGpioLedWanAct,         .u.us = BP_GPIO_31_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH1 | BP_AFE_LD_6305 | BP_AFE_FE_REV_6305_REV_12_5_60_1},
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_35_AH}, // Line Driver 1 = "Ext"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_36_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_37_AH},
  {bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_18_AH},
  {bp_usGpioAFELDRelay,        .u.us = BP_GPIO_3_AH},
  {bp_usIntfEnd},
//secondary definitions for AfeIds
  {bp_usIntfId,                .u.us = 11},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_60},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 12},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 1},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH1 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_60},
  {bp_usIntfEnd},

  {bp_usIntfId,                .u.us = 8},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xPON},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_26_AH},
  {bp_InvSerdesRxPol,          .u.us = pmd_polarity_invert},
  {bp_InvSerdesTxPol,          .u.us = pmd_polarity_invert},
  {bp_usRogueOnuEn,            .u.us = BP_GPIO_40_AH},
  {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
  {bp_usIntfMgmtBusNum,        .u.us = 0},
  {bp_usSfpSigDetect,          .u.us = BP_GPIO_10_AH },
  {bp_usPmdMACEwakeEn,         .u.us = BP_GPIO_39_AH},
  {bp_usExtIntrPmdAlarm,       .u.us = BP_EXT_INTR_3 | BP_EXT_INTR_TYPE_IRQ_HIGH_LEVEL | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_40_AH},
  {bp_usGpioPmdReset,          .u.us = BP_GPIO_5_AL},
  {bp_usGpioLedSim,            .u.us = BP_GPIO_NONE},
  {bp_pmdFunc,                 .u.us = BP_PMD_APD_TYPE_BOOST | BP_PMD_APD_REG_DISABLED},
  {bp_usIntfEnd},

  {bp_elemTemplate,            .u.bp_elemp = g_bcm963158ref3},
  {bp_last}
};

static bp_elem_t g_bcm963158x_2p5glan[] = {
  {bp_cpBoardId,               .u.cp = "963158X_2P5GLAN"},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND}, 
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_EMMC}, 
  {bp_usButtonIdx,             .u.us = 0},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {  bp_usGpio_Intr,           .u.us = BP_GPIO_46_AL},
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {      bp_ulButtonActionParm, .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},


  {bp_usGphyBaseAddress,       .u.us = BCM963158_PHY_BASE},  // use phy addresses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x01},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ulPhyId5,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_WAN_ONLY },
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x50},
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10}, /* GPHY4 */
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963158_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId6,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulCrossbarPhyId,         .u.ul = 0x1e | PHY_EXTERNAL | PHY_TYPE_CL45GPHY},

  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 4},
  {bp_usNetLed0,               .u.us = BP_GPIO_72_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_GPIO_73_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_GPIO_88_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
  {bp_usPortNum,               .u.us = SF2_WAN_PORT_NUM},
  {bp_ulCrossbar,              .u.ul = 9},  /* SGMII on EtherWAN port */
  {bp_usNetLed0,               .u.us = BP_GPIO_74_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_GPIO_75_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed2,               .u.us = BP_GPIO_76_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500},
  {bp_usNetLed3,               .u.us = BP_GPIO_89_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
  {bp_usIntfMgmtBusNum,        .u.us = 1},
  {bp_usExtIntrOpticalModulePresence, .u.us = BP_EXT_INTR_2 | BP_EXT_INTR_TYPE_IRQ_BOTH_EDGE | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_20_AL},
  {bp_usGpioSfpModDetect,      .u.us = BP_GPIO_20_AL}, 
  /*  GPIO 19 is used by vregsync in this board. To use SGIMII, must disable
      vregsync and enable this line */
  /* {bp_usSfpSigDetect,          .u.us = BP_GPIO_19_AH },*/
  {bp_usIntfEnd},

  {bp_usIntfId,                .u.us = 2},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_GPIO_18_AH},
  {bp_usGpioLedWanAct,         .u.us = BP_GPIO_94_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6305 | BP_AFE_FE_REV_6305_REV_12_5_60_2},
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_32_AH}, // Line Driver 0 = "Int"
  {bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_11_AH},
  {bp_usGpioAFELDRelay,        .u.us = BP_GPIO_38_AH},
  {bp_usVregSync,              .u.us = BP_GPIO_19_AH},
  {bp_usIntfEnd},

  {bp_usIntfId,                .u.us = 3},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xPON},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_GPIO_90_AH|BP_LED_USE_GPIO},
  {bp_InvSerdesRxPol,          .u.us = pmd_polarity_invert},
  {bp_InvSerdesTxPol,          .u.us = pmd_polarity_invert},
  {bp_usRogueOnuEn,            .u.us = BP_GPIO_40_AH},
  {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
  {bp_usIntfMgmtBusNum,        .u.us = 0},
  {bp_usSfpSigDetect,          .u.us = BP_GPIO_10_AH },
  {bp_usPmdMACEwakeEn,         .u.us = BP_GPIO_33_AH},
  {bp_usExtIntrPmdAlarm,       .u.us = BP_EXT_INTR_3 | BP_EXT_INTR_TYPE_IRQ_HIGH_LEVEL | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_40_AH},
  {bp_usGpioPmdReset,          .u.us = BP_GPIO_5_AL},
  {bp_pmdFunc,                 .u.us = BP_PMD_APD_REG_DISABLED},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 4},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioI2cSda,            .u.us = BP_GPIO_24_AH }, /* i2c for pon optical module */
  {bp_usGpioI2cScl,            .u.us = BP_GPIO_25_AH },
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 5},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
  {bp_usPortNum,               .u.us = 1},
  {bp_usGpioI2cSda,            .u.us = BP_GPIO_15_AH }, /* i2c for sgmii optical module */
  {bp_usGpioI2cScl,            .u.us = BP_GPIO_16_AH },
  {bp_usIntfEnd},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_1067_14_14_14 | BP_DDR_TOTAL_SIZE_1024MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_32BIT | BP_DDR_SSC_CONFIG_1},

  {bp_last}
};

static bp_elem_t g_bcm963158x_2p5gwan[] = {
  {bp_cpBoardId,               .u.cp = "963158X_2P5GWAN"},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND}, 
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_EMMC}, 
  {bp_usButtonIdx,             .u.us = 0},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {  bp_usGpio_Intr,           .u.us = BP_GPIO_46_AL},
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {      bp_ulButtonActionParm, .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},


  {bp_usGphyBaseAddress,       .u.us = BCM963158_PHY_BASE},  // use phy addresses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x21},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ulPhyId5,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_WAN_ONLY },
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulCrossbarPhyId,         .u.ul = 0x1e | PHY_EXTERNAL | PHY_TYPE_CL45GPHY},
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x10},
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10}, /* GPHY4 */
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963158_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},

  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 4},
  {bp_usNetLed0,               .u.us = BP_GPIO_72_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_GPIO_73_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed3,               .u.us = BP_GPIO_88_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
  {bp_usPortNum,               .u.us = SF2_WAN_PORT_NUM},
  {bp_ulCrossbar,              .u.ul = 9},  /* SGMII on EtherWAN port */
  {bp_usNetLed0,               .u.us = BP_GPIO_74_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_GPIO_75_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed2,               .u.us = BP_GPIO_76_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500},
  {bp_usNetLed3,               .u.us = BP_GPIO_89_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
  {bp_usIntfMgmtBusNum,        .u.us = 1},
  {bp_usExtIntrOpticalModulePresence, .u.us = BP_EXT_INTR_2 | BP_EXT_INTR_TYPE_IRQ_BOTH_EDGE | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_20_AL},
  {bp_usGpioSfpModDetect,      .u.us = BP_GPIO_20_AL}, 
  /*  GPIO 19 is used by vregsync in this board. To use SGIMII, must disable
      vregsync and enable this line */
  /* {bp_usSfpSigDetect,          .u.us = BP_GPIO_19_AH },*/
  {bp_usIntfEnd},

  {bp_usIntfId,                .u.us = 2},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_GPIO_18_AH},
  {bp_usGpioLedWanAct,         .u.us = BP_GPIO_94_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6305 | BP_AFE_FE_REV_6305_REV_12_5_60_2},
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_32_AH}, // Line Driver 0 = "Int"
  {bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_11_AH},
  {bp_usGpioAFELDRelay,        .u.us = BP_GPIO_38_AH},
  {bp_usVregSync,              .u.us = BP_GPIO_19_AH},
  {bp_usIntfEnd},

  {bp_usIntfId,                .u.us = 3},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xPON},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_GPIO_90_AH|BP_LED_USE_GPIO},
  {bp_InvSerdesRxPol,          .u.us = pmd_polarity_invert},
  {bp_InvSerdesTxPol,          .u.us = pmd_polarity_invert},
  {bp_usRogueOnuEn,            .u.us = BP_GPIO_40_AH},
  {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
  {bp_usIntfMgmtBusNum,        .u.us = 0},
  {bp_usSfpSigDetect,          .u.us = BP_GPIO_10_AH },
  {bp_usPmdMACEwakeEn,         .u.us = BP_GPIO_33_AH},
  {bp_usExtIntrPmdAlarm,       .u.us = BP_EXT_INTR_3 | BP_EXT_INTR_TYPE_IRQ_HIGH_LEVEL | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_40_AH},
  {bp_usGpioPmdReset,          .u.us = BP_GPIO_5_AL},
  {bp_pmdFunc,                 .u.us = BP_PMD_APD_REG_DISABLED},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 4},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioI2cSda,            .u.us = BP_GPIO_24_AH }, /* i2c for pon optical module */
  {bp_usGpioI2cScl,            .u.us = BP_GPIO_25_AH },
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 5},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
  {bp_usPortNum,               .u.us = 1},
  {bp_usGpioI2cSda,            .u.us = BP_GPIO_15_AH }, /* i2c for sgmii optical module */
  {bp_usGpioI2cScl,            .u.us = BP_GPIO_16_AH },
  {bp_usIntfEnd},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_1067_14_14_14 | BP_DDR_TOTAL_SIZE_1024MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_32BIT | BP_DDR_SSC_CONFIG_1},

  {bp_last}
};

static bp_elem_t g_bcm963158x_54991[] = {
  {bp_cpBoardId,               .u.cp = "963158X_54991"},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND}, 
  {bp_usButtonIdx,             .u.us = 0},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {  bp_usGpio_Intr,           .u.us = BP_GPIO_46_AL},
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {      bp_ulButtonActionParm, .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},


  {bp_usGphyBaseAddress,       .u.us = BCM963158_PHY_BASE},  // use phy addresses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x11},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ulPhyId4,                .u.ul = 0|MAC_IF_XGAE_SERDES},  /* Dummy PHY ID */
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x40},
  {bp_ulPhyId6,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulCrossbarPhyId,         .u.ul = 0x1f | PHY_EXTERNAL | PHY_TYPE_CL45GPHY},


  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
  {bp_usPortNum,               .u.us = 6},
  {bp_ulCrossbar,              .u.ul = 9},  /* SGMII on LAN port */
  {bp_usNetLed0,               .u.us = BP_GPIO_74_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_GPIO_75_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
  {bp_usNetLed2,               .u.us = BP_GPIO_76_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500},
  {bp_usNetLed3,               .u.us = BP_GPIO_89_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},

  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xPON},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_GPIO_90_AH},
  {bp_usNetLed0,               .u.us = BP_GPIO_77_AH},  /* For AE hardware led */
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_usNetLed1,               .u.us = BP_GPIO_78_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed2,               .u.us = BP_GPIO_79_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500|BP_NET_LED_SPEED_10G},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_SPEED_2500|BP_NET_LED_SPEED_10G},
  {bp_usExtIntrOpticalModulePresence, .u.us = BP_EXT_INTR_2 | BP_EXT_INTR_TYPE_IRQ_BOTH_EDGE | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_9_AL},
  {bp_usGpioSfpModDetect,      .u.us = BP_GPIO_9_AL},
  {bp_usGpioPonTxEn,           .u.us = BP_GPIO_3_AH},
  {bp_usRogueOnuEn,            .u.us = BP_GPIO_40_AH},
  {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
  {bp_usIntfMgmtBusNum,        .u.us = 1},
  {bp_usSfpSigDetect,          .u.us = BP_GPIO_10_AH },
  {bp_usIntfEnd},


  {bp_usIntfId,                .u.us = 2},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_GPIO_18_AH},
  {bp_usGpioLedWanAct,         .u.us = BP_GPIO_94_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6305 | BP_AFE_FE_REV_6305_REV_12_5_60_2|BP_AFE_FE_RNC},
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_32_AH}, // Line Driver 0 = "Int"
  {bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_11_AH},
  {bp_usGpioAFELDRelay,        .u.us = BP_GPIO_38_AH},
  {bp_usVregSync,              .u.us = BP_GPIO_19_AH},
  {bp_usIntfEnd},

  {bp_usIntfId,                .u.us = 3},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
  {bp_usPortNum,               .u.us = 1},
  {bp_usGpioI2cSda,            .u.us = BP_GPIO_15_AH }, /* i2c for sgmii optical module */
  {bp_usGpioI2cScl,            .u.us = BP_GPIO_16_AH },
  {bp_usIntfEnd},

  {bp_ulMemoryConfig,          .u.ul =  BP_DDR_SPEED_1067_14_14_14 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_SSC_CONFIG_1},

  {bp_last}
};

static bp_elem_t g_bcm963158x_84881[] = {
  {bp_cpBoardId,               .u.cp = "963158X_84881"},

  {bp_usGphyBaseAddress,       .u.us = BCM963158_PHY_BASE},  // use phy addresses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x11},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ulPhyId4,                .u.ul = 0|MAC_IF_XGAE_SERDES},  /* Dummy PHY ID */
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x40},
  {bp_ulPhyId6,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulCrossbarPhyId,         .u.ul = 0x1e | PHY_EXTERNAL | PHY_TYPE_CL45GPHY},

  {bp_elemTemplate,            .u.bp_elemp = g_bcm963158x_54991},

  {bp_last}
};

static bp_elem_t g_bcm963158ref3d[] = {
  {bp_cpBoardId,               .u.cp = "963158REF3D"},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_EMMC},
  {bp_usSerialLedData,         .u.us = BP_GPIO_0_AH},
  {bp_usSerialLedClk,          .u.us = BP_GPIO_1_AH},
  {bp_usSerialLedMask,         .u.us = BP_GPIO_2_AH},
  /* Any serial led that are assigned in the hardware design but not used by board parameter 
     should reserve one entry below to make sure other serial led output in the right slot*/
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_16_AH},
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_17_AH},
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_27_AH},

  {bp_usButtonIdx,             .u.us = 0},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {  bp_usGpio_Intr,           .u.us = BP_GPIO_46_AL},
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {      bp_ulButtonActionParm, .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},
  {bp_usButtonIdx,             .u.us = 1},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_1|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {  bp_usGpio_Intr,           .u.us = BP_GPIO_38_AL},
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_29_AH},
  {bp_usSpiSlaveSelectNum,     .u.us = 1},
  {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_112_AL},

  {bp_usGphyBaseAddress,       .u.us = BCM963158_PHY_BASE},  // use phy addresses on SF2 with base address 0x8
#if defined(CONFIG_BCM_SYSPORT)
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
#else
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x21},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ulPhyId5,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963158_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
#endif
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = (BCM963158_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963158_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963158_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963158_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
#if 0 /* for RGMII daughter card */
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 12},
  {bp_ulCrossbarPhyId,         .u.ul = 0x18 | PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},  /* make sure the phy id matches the one on the plug in rgmii phy daughter card */
#endif

  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_GPIO_18_AH},
  {bp_usGpioLedWanAct,         .u.us = BP_GPIO_30_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6305 | BP_AFE_FE_REV_6305_REV_12_5_60_2|BP_AFE_FE_RNC},
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_32_AH}, // Line Driver 0 = "Int"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_33_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_34_AH},
  {bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_11_AH},
  {bp_usGpioAFELDRelay,        .u.us = BP_GPIO_22_AH},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 1},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_19_AH},
  {bp_usGpioLedWanAct,         .u.us = BP_GPIO_31_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH1 | BP_AFE_LD_6305 | BP_AFE_FE_REV_6305_REV_12_5_60_1|BP_AFE_FE_RNC},
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_35_AH}, // Line Driver 1 = "Ext"
  {bp_usGpioAFELDData,         .u.us = BP_GPIO_36_AH},
  {bp_usGpioAFELDClk,          .u.us = BP_GPIO_37_AH},
  {bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_18_AH},
  {bp_usGpioAFELDRelay,        .u.us = BP_GPIO_3_AH},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 2},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 0},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_0_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_1_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_20_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 3},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 1},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_2_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_3_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_21_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 4},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 2},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_4_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_5_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_22_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 5},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 3},
  {bp_usNetLed0,               .u.us = BP_GPIO_6_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_GPIO_7_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_GPIO_23_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 6},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = SF2_WAN_PORT_NUM}, /* SGPHY on EtherWAN port */
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_8_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_9_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_24_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},

  {bp_usIntfId,                .u.us = 7},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
  {bp_usPortNum,               .u.us = SF2_WAN_PORT_NUM},
  {bp_ulCrossbar,              .u.ul = 9},  /* SGMII on EtherWAN port */
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_10_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_11_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed2,               .u.us = BP_SERIAL_GPIO_12_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_25_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
  {bp_usIntfMgmtBusNum,        .u.us = 1},
  {bp_usExtIntrOpticalModulePresence, .u.us = BP_EXT_INTR_2 | BP_EXT_INTR_TYPE_IRQ_BOTH_EDGE | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_20_AL},
  {bp_usGpioSfpModDetect,      .u.us = BP_GPIO_20_AL},
  {bp_usSfpSigDetect,         .u.us = BP_GPIO_21_AL },
  {bp_usIntfEnd},

  {bp_usIntfId,                .u.us = 8},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xPON},
  {bp_usPortNum,               .u.us = 0},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_13_AH},  /* For AE hardware led */
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_14_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed2,               .u.us = BP_SERIAL_GPIO_15_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500|BP_NET_LED_SPEED_10G},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_SPEED_2500|BP_NET_LED_SPEED_10G},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_26_AH},
  {bp_InvSerdesRxPol,          .u.us = pmd_polarity_invert},
  {bp_InvSerdesTxPol,          .u.us = pmd_polarity_invert},
  {bp_usRogueOnuEn,            .u.us = BP_GPIO_40_AH},
  {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
  {bp_usIntfMgmtBusNum,        .u.us = 0},
  {bp_usSfpSigDetect,          .u.us = BP_GPIO_10_AH },
  {bp_usPmdMACEwakeEn,         .u.us = BP_GPIO_39_AH},
  {bp_usExtIntrPmdAlarm,       .u.us = BP_EXT_INTR_3 | BP_EXT_INTR_TYPE_IRQ_HIGH_LEVEL | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_40_AH},
  {bp_usGpioPmdReset,          .u.us = BP_GPIO_5_AL},
  {bp_usGpioLedSim,            .u.us = BP_GPIO_NONE},
  {bp_pmdFunc,                 .u.us = BP_PMD_APD_TYPE_BOOST | BP_PMD_APD_REG_DISABLED},
  {bp_usIntfEnd},

  {bp_usIntfId,                .u.us = 9},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioI2cSda,            .u.us = BP_GPIO_24_AH }, /* i2c for pon optical module */
  {bp_usGpioI2cScl,            .u.us = BP_GPIO_25_AH },
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 10},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
  {bp_usPortNum,               .u.us = 1},
  {bp_usGpioI2cSda,            .u.us = BP_GPIO_15_AH }, /* i2c for sgmii optical module */
  {bp_usGpioI2cScl,            .u.us = BP_GPIO_16_AH },
  {bp_usIntfEnd},
//secondary definitions for AfeIds
  {bp_usIntfId,                .u.us = 11},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_60},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 12},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 1},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH1 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_60},
  {bp_usIntfEnd},

  {bp_usUsbPwrOn1,             .u.us = BP_GPIO_124_AL},
  {bp_usUsbPwrFlt1,            .u.us = BP_GPIO_123_AL},
  {bp_usUsbPwrOn0,             .u.us = BP_GPIO_122_AL},
  {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_121_AL},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_GPIO_28_AL},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_1067_14_14_14 | BP_DDR_TOTAL_SIZE_1024MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_32BIT | BP_DDR_SSC_CONFIG_1},
#if !defined(_CFE_)
  {bp_usGpioPwrSync,           .u.us = BP_GPIO_8_AL},
  {bp_usVregSync,              .u.us = BP_GPIO_19_AH},
#endif
  {bp_ulPciFlags,              .u.ul = BP_PCI0_DUAL_LANE},
  {bp_usGpioWireless0Disable,     .u.us = BP_GPIO_86_AH},
  {bp_usGpioWireless2Disable,     .u.us = BP_GPIO_87_AH},
  {bp_usGpioWireless3Disable,     .u.us = BP_GPIO_88_AH},
  {bp_last}
};

static bp_elem_t g_bcm963158ref3d_p200[] = {
  {bp_cpBoardId,               .u.cp = "158REF3D_P200"},

  {bp_usIntfId,                .u.us = 8},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xPON},
  {bp_usPortNum,               .u.us = 0},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_13_AH},  /* For AE hardware led */
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_14_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed2,               .u.us = BP_SERIAL_GPIO_15_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500|BP_NET_LED_SPEED_10G},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_SPEED_2500|BP_NET_LED_SPEED_10G},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_26_AH},
  {bp_InvSerdesRxPol,          .u.us = pmd_polarity_invert},
  {bp_InvSerdesTxPol,          .u.us = pmd_polarity_invert},
  {bp_usRogueOnuEn,            .u.us = BP_GPIO_40_AH},
  {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
  {bp_usIntfMgmtBusNum,        .u.us = 0},
  {bp_usSfpSigDetect,          .u.us = BP_GPIO_10_AH },
  {bp_usPmdMACEwakeEn,         .u.us = BP_GPIO_41_AH},  /* update from gpio 39 to 41 in P200 */
  {bp_usExtIntrPmdAlarm,       .u.us = BP_EXT_INTR_3 | BP_EXT_INTR_TYPE_IRQ_HIGH_LEVEL | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_40_AH},
  {bp_usGpioPmdReset,          .u.us = BP_GPIO_5_AL},
  {bp_usGpioLedSim,            .u.us = BP_GPIO_NONE},
  {bp_pmdFunc,                 .u.us = BP_PMD_APD_TYPE_BOOST | BP_PMD_APD_REG_DISABLED},
  {bp_usIntfEnd},

  {bp_elemTemplate,            .u.bp_elemp = g_bcm963158ref3d},
  {bp_last}
};


static bp_elem_t g_bcm963158ref2_2p5glan[] = {
  {bp_cpBoardId,               .u.cp = "963158REF2_2P5L"},
 
  {bp_usGphyBaseAddress,       .u.us = BCM963158_PHY_BASE},  // use phy addresses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x01},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x5f},
  {bp_ulPhyId0,                .u.ul = (BCM963158_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963158_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963158_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963158_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963158_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId6,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},

  {bp_elemTemplate,            .u.bp_elemp = g_bcm963158ref2},
  {bp_last}
};

static bp_elem_t g_bcm963158ref3_2p5glan[] = {
  {bp_cpBoardId,               .u.cp = "963158REF3_2P5L"},
 
  {bp_usGphyBaseAddress,       .u.us = BCM963158_PHY_BASE},  // use phy addresses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x01},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x5f},
  {bp_ulPhyId0,                .u.ul = (BCM963158_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963158_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963158_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963158_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963158_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId6,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},

  {bp_elemTemplate,            .u.bp_elemp = g_bcm963158ref3},
  {bp_last}
};

static bp_elem_t g_bcm963158ref2d[] = {
  {bp_cpBoardId,               .u.cp = "963158REF2D"},
#if !defined(_CFE_)
  {bp_usGpioPwrSync,           .u.us = BP_GPIO_91_AL},
  {bp_usVregSync,              .u.us = BP_GPIO_19_AH},
#endif
  {bp_usIntfId,                .u.us = 7},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
  {bp_usPortNum,               .u.us = SF2_WAN_PORT_NUM},
  {bp_ulCrossbar,              .u.ul = 9},  /* SGMII on EtherWAN port */
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_10_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_11_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed2,               .u.us = BP_SERIAL_GPIO_12_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_25_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
  {bp_usIntfMgmtBusNum,        .u.us = 1},
  {bp_usExtIntrOpticalModulePresence, .u.us = BP_EXT_INTR_3 | BP_EXT_INTR_TYPE_IRQ_BOTH_EDGE | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_20_AL},   /* update the sfp detection pin for 2d board */
  {bp_usGpioSfpModDetect,      .u.us = BP_GPIO_20_AL},
  {bp_usSfpSigDetect,          .u.us = BP_GPIO_21_AL },
  {bp_usIntfEnd},
  {bp_usGpioWireless0Disable,     .u.us = BP_GPIO_98_AH},
  {bp_usGpioWireless2Disable,     .u.us = BP_GPIO_99_AH},
  {bp_usGpioWireless3Disable,     .u.us = BP_GPIO_100_AH},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963158ref2},
  {bp_last}
};

static bp_elem_t g_bcm963158ref2d_2p5glan[] = {
  {bp_cpBoardId,               .u.cp = "158REF2D_2P5L"},
 
  {bp_usGphyBaseAddress,       .u.us = BCM963158_PHY_BASE},  // use phy addresses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x01},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x5f},
  {bp_ulPhyId0,                .u.ul = (BCM963158_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963158_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963158_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963158_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963158_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId6,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},

  {bp_elemTemplate,            .u.bp_elemp = g_bcm963158ref2d},
  {bp_last}
};


static bp_elem_t g_bcm963153ref4d_xg[] = {
  {bp_cpBoardId,               .u.cp = "963153REF4D_XG"},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_EMMC},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_I2S},  // Enable I2S pinmux
  {bp_usSerialLedData,         .u.us = BP_GPIO_0_AH},
  {bp_usSerialLedClk,          .u.us = BP_GPIO_1_AH},
  {bp_usSerialLedMask,         .u.us = BP_GPIO_2_AH},
  {bp_usGpioPonTxEn,           .u.us = BP_GPIO_3_AH},
  /* Any serial led that are assigned in the hardware design but not used by board parameter 
     should reserve one entry below to make sure other serial led output in the right slot*/
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_8_AH},
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_9_AH},
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_16_AH},
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_17_AH},
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_24_AH},
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_27_AH},

  {bp_usButtonIdx,             .u.us = 0},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {  bp_usGpio_Intr,           .u.us = BP_GPIO_46_AL},
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {      bp_ulButtonActionParm, .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},
  {bp_usButtonIdx,             .u.us = 1},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_1|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {  bp_usGpio_Intr,           .u.us = BP_GPIO_38_AL},
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },

  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_29_AH},
  {bp_usSpiSlaveSelectNum,     .u.us = 1},
  {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_112_AL},
#if !defined(_CFE_)
  {bp_usGpioPwrSync,           .u.us = BP_GPIO_8_AL},
  {bp_usVregSync,              .u.us = BP_GPIO_19_AH},
#endif
  {bp_usI2sRxSdata,            .u.us = BP_GPIO_14_AH},
  {bp_usI2sMclk,               .u.us = BP_GPIO_28_AH},
  {bp_usI2sLrck,               .u.us = BP_GPIO_29_AH},
  {bp_usI2sSclk,               .u.us = BP_GPIO_30_AH},
  {bp_usI2sSdata,              .u.us = BP_GPIO_31_AH},
  {bp_usPcmSdin,               .u.us = BP_GPIO_42_AH},
  {bp_usPcmSdout,              .u.us = BP_GPIO_43_AH},
  {bp_usPcmClk,                .u.us = BP_GPIO_44_AH},
  {bp_usPcmFs,                 .u.us = BP_GPIO_45_AH},

  {bp_usGphyBaseAddress,       .u.us = BCM963158_PHY_BASE},  // use phy addresses on SF2 with base address 0x8
#if defined(CONFIG_BCM_SYSPORT)
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
#else
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x11},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ulPhyId4,                .u.ul = 0|MAC_IF_XGAE_SERDES},  /* Dummy PHY ID */
  {bp_ulPhyId4,                .u.ul = 0x1f | PHY_EXTERNAL | PHY_TYPE_CL45GPHY},
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
#endif
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = (BCM963158_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963158_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963158_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963158_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},

  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_GPIO_18_AH},
  {bp_usGpioLedWanAct,         .u.us = BP_GPIO_30_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6305 | BP_AFE_FE_REV_6305_REV_12_5_60_2|BP_AFE_FE_RNC},
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_32_AH}, // Line Driver 0 = "Int"
  {bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_11_AH},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 1},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_19_AH},
  {bp_usGpioLedWanAct,         .u.us = BP_GPIO_31_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH1 | BP_AFE_LD_6305 | BP_AFE_FE_REV_6305_REV_12_5_60_1|BP_AFE_FE_RNC},
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_35_AH}, // Line Driver 1 = "Ext"
  {bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_11_AH},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 2},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 0},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_0_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_1_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_20_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 3},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 1},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_2_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_3_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_21_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 4},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 2},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_4_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_5_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_22_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 5},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 3},
  {bp_usNetLed0,               .u.us = BP_GPIO_6_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_GPIO_7_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_GPIO_23_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 6},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
  {bp_usPortNum,               .u.us = SF2_WAN_PORT_NUM},
  {bp_ulCrossbar,              .u.ul = 9},  /* SGMII on EtherWAN port */
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_10_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_11_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed2,               .u.us = BP_SERIAL_GPIO_12_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_25_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 7},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xPON},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_26_AH},  /* For GPON ranging software led */
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_13_AH},  /* For AE hardware led */
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_14_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed2,               .u.us = BP_SERIAL_GPIO_15_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500|BP_NET_LED_SPEED_10G},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_SPEED_2500|BP_NET_LED_SPEED_10G},
  {bp_InvSerdesRxPol,          .u.us = pmd_polarity_invert},
  {bp_InvSerdesTxPol,          .u.us = pmd_polarity_invert},
  {bp_usRogueOnuEn,            .u.us = BP_GPIO_40_AH},
  {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
  {bp_usIntfMgmtBusNum,        .u.us = 0},
  {bp_usSfpSigDetect,          .u.us = BP_GPIO_10_AH },
  {bp_usPmdMACEwakeEn,         .u.us = BP_GPIO_41_AH},
  {bp_usExtIntrPmdAlarm,       .u.us = BP_EXT_INTR_2 | BP_EXT_INTR_TYPE_IRQ_HIGH_LEVEL | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_40_AH},
  {bp_usGpioPmdReset,          .u.us = BP_GPIO_5_AL},
  {bp_pmdFunc,                 .u.us = BP_PMD_APD_REG_DISABLED},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 8},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioI2cSda,            .u.us = BP_GPIO_24_AH }, /* i2c for pon optical module */
  {bp_usGpioI2cScl,            .u.us = BP_GPIO_25_AH },
  {bp_usIntfEnd},
#if 0 /* for SGMII daughter card */
  {bp_usIntfId,                .u.us = 9},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
  {bp_usPortNum,               .u.us = 1},
  {bp_usGpioI2cSda,            .u.us = BP_GPIO_15_AH }, /* i2c for sgmii optical module */
  {bp_usGpioI2cScl,            .u.us = BP_GPIO_16_AH },
  {bp_usIntfEnd},
#endif

  {bp_usUsbPwrOn1,             .u.us = BP_GPIO_124_AL},
  {bp_usUsbPwrFlt1,            .u.us = BP_GPIO_123_AL},
  {bp_usUsbPwrOn0,             .u.us = BP_GPIO_122_AL},
  {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_121_AL},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_GPIO_28_AH},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_1067_14_14_14 | BP_DDR_TOTAL_SIZE_1024MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_32BIT | BP_DDR_SSC_CONFIG_1},
  {bp_ulPciFlags,              .u.ul = BP_PCI0_DUAL_LANE},
  {bp_usGpioWireless0Disable,     .u.us = BP_GPIO_98_AH},
  {bp_usGpioWireless2Disable,     .u.us = BP_GPIO_99_AH},
  {bp_usGpioWireless3Disable,     .u.us = BP_GPIO_100_AH},
  {bp_last}
};

static bp_elem_t g_bcm963153ref4d[] = {
  {bp_cpBoardId,               .u.cp = "963153REF4D"},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_EMMC},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_I2S},  // Enable I2S pinmux
  {bp_usSerialLedData,         .u.us = BP_GPIO_0_AH},
  {bp_usSerialLedClk,          .u.us = BP_GPIO_1_AH},
  {bp_usSerialLedMask,         .u.us = BP_GPIO_2_AH},
  /* Any serial led that are assigned in the hardware design but not used by board parameter 
     should reserve one entry below to make sure other serial led output in the right slot*/
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_8_AH},
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_9_AH},
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_16_AH},
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_17_AH},
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_24_AH},
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_27_AH},

  {bp_usButtonIdx,             .u.us = 0},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {  bp_usGpio_Intr,           .u.us = BP_GPIO_46_AL},
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {      bp_ulButtonActionParm, .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},
  {bp_usButtonIdx,             .u.us = 1},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_1|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {  bp_usGpio_Intr,           .u.us = BP_GPIO_38_AL},
  {    bp_usButtonAction,      .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },

  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_29_AH},
  {bp_usSpiSlaveSelectNum,     .u.us = 1},
  {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_112_AL},
#if !defined(_CFE_)
  {bp_usGpioPwrSync,           .u.us = BP_GPIO_8_AL},
  {bp_usVregSync,              .u.us = BP_GPIO_19_AH},
#endif
  {bp_usI2sRxSdata,            .u.us = BP_GPIO_14_AH},
  {bp_usI2sMclk,               .u.us = BP_GPIO_28_AH},
  {bp_usI2sLrck,               .u.us = BP_GPIO_29_AH},
  {bp_usI2sSclk,               .u.us = BP_GPIO_30_AH},
  {bp_usI2sSdata,              .u.us = BP_GPIO_31_AH},
  {bp_usPcmSdin,               .u.us = BP_GPIO_42_AH},
  {bp_usPcmSdout,              .u.us = BP_GPIO_43_AH},
  {bp_usPcmClk,                .u.us = BP_GPIO_44_AH},
  {bp_usPcmFs,                 .u.us = BP_GPIO_45_AH},

  {bp_usGphyBaseAddress,       .u.us = BCM963158_PHY_BASE},  // use phy addresses on SF2 with base address 0x8
#if defined(CONFIG_BCM_SYSPORT)
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
#else
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x21},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ulPhyId5,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_WAN_ONLY },
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulCrossbarPhyId,         .u.ul = 0x1f | PHY_EXTERNAL | PHY_TYPE_CL45GPHY},
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
#endif
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = (BCM963158_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963158_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963158_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963158_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},

  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_GPIO_18_AH},
  {bp_usGpioLedWanAct,         .u.us = BP_GPIO_30_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6305 | BP_AFE_FE_REV_6305_REV_12_5_60_2|BP_AFE_FE_RNC},
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_32_AH}, // Line Driver 0 = "Int"
  {bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_11_AH},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
  {bp_usPortNum,               .u.us = 1},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_19_AH},
  {bp_usGpioLedWanAct,         .u.us = BP_GPIO_31_AH},
  {bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH1 | BP_AFE_LD_6305 | BP_AFE_FE_REV_6305_REV_12_5_60_1|BP_AFE_FE_RNC},
  {bp_usGpioAFELDPwr,          .u.us = BP_GPIO_35_AH}, // Line Driver 1 = "Ext"
  {bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_11_AH},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 2},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 0},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_0_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_1_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_20_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 3},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 1},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_2_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_3_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_21_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 4},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 2},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_4_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_5_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_22_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 5},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
  {bp_usPortNum,               .u.us = 3},
  {bp_usNetLed0,               .u.us = BP_GPIO_6_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100},
  {bp_usNetLed1,               .u.us = BP_GPIO_7_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed3,               .u.us = BP_GPIO_23_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 6},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
  {bp_usPortNum,               .u.us = SF2_WAN_PORT_NUM},
  {bp_ulCrossbar,              .u.ul = 9},  /* SGMII on EtherWAN port */
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_10_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_11_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed2,               .u.us = BP_SERIAL_GPIO_12_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_25_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 7},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_xPON},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioLedWanLink,        .u.us = BP_SERIAL_GPIO_26_AH},  /* For GPON ranging software led */
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_13_AH},  /* For AE hardware led */
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_14_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed2,               .u.us = BP_SERIAL_GPIO_15_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500|BP_NET_LED_SPEED_10G},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_SPEED_2500|BP_NET_LED_SPEED_10G},
  {bp_InvSerdesRxPol,          .u.us = pmd_polarity_invert},
  {bp_InvSerdesTxPol,          .u.us = pmd_polarity_invert},
  {bp_usRogueOnuEn,            .u.us = BP_GPIO_40_AH},
  {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
  {bp_usIntfMgmtBusNum,        .u.us = 0},
  {bp_usSfpSigDetect,          .u.us = BP_GPIO_10_AH },
  {bp_usPmdMACEwakeEn,         .u.us = BP_GPIO_41_AH},
  {bp_usExtIntrPmdAlarm,       .u.us = BP_EXT_INTR_2 | BP_EXT_INTR_TYPE_IRQ_HIGH_LEVEL | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_40_AH},
  {bp_usGpioPmdReset,          .u.us = BP_GPIO_5_AL},
  {bp_pmdFunc,                 .u.us = BP_PMD_APD_REG_DISABLED},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 8},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioI2cSda,            .u.us = BP_GPIO_24_AH }, /* i2c for pon optical module */
  {bp_usGpioI2cScl,            .u.us = BP_GPIO_25_AH },
  {bp_usIntfEnd},
#if 0 /* for SGMII daughter card */
  {bp_usIntfId,                .u.us = 9},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
  {bp_usPortNum,               .u.us = 1},
  {bp_usGpioI2cSda,            .u.us = BP_GPIO_15_AH }, /* i2c for sgmii optical module */
  {bp_usGpioI2cScl,            .u.us = BP_GPIO_16_AH },
  {bp_usIntfEnd},
#endif

  {bp_usUsbPwrOn1,             .u.us = BP_GPIO_124_AL},
  {bp_usUsbPwrFlt1,            .u.us = BP_GPIO_123_AL},
  {bp_usUsbPwrOn0,             .u.us = BP_GPIO_122_AL},
  {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_121_AL},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_GPIO_28_AH},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_1067_14_14_14 | BP_DDR_TOTAL_SIZE_1024MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_32BIT | BP_DDR_SSC_CONFIG_1},
  {bp_ulPciFlags,              .u.ul = BP_PCI0_DUAL_LANE},
  {bp_usGpioWireless0Disable,     .u.us = BP_GPIO_98_AH},
  {bp_usGpioWireless2Disable,     .u.us = BP_GPIO_99_AH},
  {bp_usGpioWireless3Disable,     .u.us = BP_GPIO_100_AH},
  {bp_last}
};

static bp_elem_t g_bcm963153ref4d_no2p5_phy[] = {   /* Creating this for non PHY parts board in lab */
  {bp_cpBoardId,               .u.cp = "153REF4D_N2P5"},

  {bp_usGphyBaseAddress,       .u.us = BCM963158_PHY_BASE},  // use phy addresses on SF2 with base address 0x8
#if defined(CONFIG_BCM_SYSPORT)
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
#else
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x01},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
#endif

  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = (BCM963158_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963158_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963158_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963158_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},

  {bp_elemTemplate,            .u.bp_elemp = g_bcm963153ref4d},
  {bp_last}
};

static bp_elem_t g_bcm963153ref4d_2p5glan[] = {
  {bp_cpBoardId,               .u.cp = "153REF4D_2P5L"},
 
  {bp_usGphyBaseAddress,       .u.us = BCM963158_PHY_BASE},  // use phy addresses on SF2 with base address 0x8
#if defined(CONFIG_BCM_SYSPORT)
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
#else
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x01},
  {bp_ulPhyId0,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
#endif
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x4f},
  {bp_ulPhyId0,                .u.ul = (BCM963158_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM963158_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM963158_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM963158_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId6,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulCrossbarPhyId,         .u.ul = 0x1f | PHY_EXTERNAL | PHY_TYPE_CL45GPHY},

  {bp_usIntfId,                .u.us = 6},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
  {bp_usPortNum,               .u.us = 6}, /* SGMII on LAN port */
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_10_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
  {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_11_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
  {bp_usNetLed2,               .u.us = BP_SERIAL_GPIO_12_AH},
  {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500},
  {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_25_AH},
  {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
  {bp_usIntfEnd},

  {bp_elemTemplate,            .u.bp_elemp = g_bcm963153ref4d},
  {bp_last}
};

bp_elem_t * g_BoardParms[] = {g_bcm963158sv, g_bcm963158dvt, g_bcm963158ref1, g_bcm963158ref2, g_bcm963158ref3, g_bcm963158dvt_lan, g_bcm963158ref3_p20x, g_bcm963158x_2p5gwan, g_bcm963158ref1_10gae, g_bcm963158ref1_10gag, g_bcm963158ref1_p20x, g_bcm963158dvt_xgae, g_bcm963158x_2p5glan, g_bcm963158ref3d, g_bcm963158ref3d_p200, g_bcm963158ref2_2p5glan, g_bcm963158ref3_2p5glan, g_bcm963158dvt_p200, g_bcm963158ref1d, g_bcm963158dvt_p200_xgae, g_bcm963153ref4d, g_bcm963153ref4d_xg, g_bcm963153ref4d_2p5glan, g_bcm963158ref2d, g_bcm963158ref2d_2p5glan, g_bcm963158ref1d_10gae, g_bcm963158ref1d_10gag, g_bcm963158x_54991, g_bcm963158x_84881, g_bcm963153ref4d_no2p5_phy, g_bcm963158ref1_2p5w, 0};


