#include "bp_defs.h"
#include "boardparms.h"
#include "bcmSpiRes.h"
extern BpCmdElem moca6802InitSeq[];


/* Only needed for internal GPHYs; by default internal GPHYs do not adv. 1000HD/100HD/10FD/10HD capabilities;
 * There are some NICs that will not negotiate 100FD - so need to advertise 100HD to link up with those NICs */
#define BCM963138_PHY_BASE             0x8

static bp_elem_t g_bcm963138sv[] = {
  {bp_cpBoardId,               .u.cp = "963138SV"},
  {bp_usGpioUart2Sdin,         .u.us = BP_GPIO_5_AH}, // uart2 is /dev/ttyS1
  {bp_usGpioUart2Sdout,        .u.us = BP_GPIO_6_AH}, // stty 115200 < /dev/ttyS1 to set speed
  {bp_usSerialLedData,         .u.us = BP_GPIO_0_AH}, // NOTE: bp_ulGpioOverlay is no longer used
  {bp_usSerialLedClk,          .u.us = BP_GPIO_1_AH},
  {bp_usSerialLedMask,         .u.us = BP_GPIO_2_AH},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND}, // Enable NAND pinmux even on SPI boot
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_40 },
  {bp_ulAfeId1,                .u.ul = BP_AFE_CHIP_CH1 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_40 },
  {bp_usGpioIntAFELDPwr,       .u.us = BP_GPIO_52_AH}, // Line Driver 0 = "Int"
  {bp_usGpioIntAFELDData,      .u.us = BP_GPIO_53_AH},
  {bp_usGpioIntAFELDClk,       .u.us = BP_GPIO_55_AH},
  {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_54_AH}, // Line Driver 1 = "Ext"
  {bp_usGpioExtAFELDData,      .u.us = BP_GPIO_9_AH},
  {bp_usGpioExtAFELDClk,       .u.us = BP_GPIO_10_AH},
  {bp_usGphyBaseAddress,       .u.us = BCM963138_PHY_BASE},  // use phy addressses on SF2 with base address 0x8

  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x3},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulPhyId1,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = (BCM963138_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_0_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_1_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_28_AL},
  {bp_ulPhyId1,                .u.ul = (BCM963138_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_3_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_11_AL},
  {bp_ulPhyId2,                .u.ul = (BCM963138_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_4_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_5_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_12_AL},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 13},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  /* BT Uart */
  {bp_usGpioUart2Cts,          .u.ul = BP_GPIO_3_AH}, 
  {bp_usGpioUart2Rts,          .u.ul = BP_GPIO_4_AH},
  {bp_usGpioUart2Sdin,         .u.ul = BP_GPIO_5_AH},
  {bp_usGpioUart2Sdout,        .u.ul = BP_GPIO_6_AH},                              
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_6_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_7_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_13_AL},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_256MB| BP_DDR_DEVICE_WIDTH_8},
  {bp_last}
};

static bp_elem_t g_bcm963138dvt[] = {
  {bp_cpBoardId,               .u.cp = "963138DVT"},
  {bp_usSerialLedData,         .u.us = BP_GPIO_0_AH}, // NOTE: bp_ulGpioOverlay is no longer used
  {bp_usSerialLedClk,          .u.us = BP_GPIO_1_AH},
  {bp_usSerialLedMask,         .u.us = BP_GPIO_2_AH},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_I2S},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND}, // Enable NAND pinmux even on SPI boot
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_29_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_30_AL},
  {bp_usGpioLedPwmReserved,    .u.us = BP_GPIO_17_AL}, // pinmux for PWM2 LED
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_15_AL},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_16_AL},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_31_AL},
  {bp_usGpioSecLedAdsl,        .u.us = BP_GPIO_18_AL},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_24_AL}, 
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_20_AL}, // LED register bit 20, shifted serially
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_40 },
  {bp_ulAfeId1,                .u.ul = BP_AFE_CHIP_CH1 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_40 },
  {bp_usGpioIntAFELDPwr,       .u.us = BP_GPIO_52_AH}, // Line Driver 0 = "Int"
  {bp_usGpioIntAFELDData,      .u.us = BP_GPIO_53_AH},
  {bp_usGpioIntAFELDClk,       .u.us = BP_GPIO_55_AH},
  {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_54_AH}, // Line Driver 1 = "Ext"
  {bp_usGpioExtAFELDData,      .u.us = BP_GPIO_9_AH},
  {bp_usGpioExtAFELDClk,       .u.us = BP_GPIO_10_AH},
  {bp_usSpiSlaveSelectNum,     .u.us = 1}, /* define the SPI select for voice */
  {bp_usSpiSlaveSelectGpioNum, .u.us = 127},
  {bp_usSpiSlaveSelectNum,     .u.us = 2},
  {bp_usSpiSlaveSelectGpioNum, .u.us = 25},
  {bp_usSpiSlaveSelectNum,     .u.us = 4},
  {bp_usSpiSlaveSelectGpioNum, .u.us = 8},
  {bp_usGphyBaseAddress,       .u.us = BCM963138_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
                                              // to avoid conflicting P0 and P11 phy address of 1
                                              // the intergrated Quad GPHY address is now 0x8, 0x9, 0xa, 0xb
                                              // the intergrated Single GPHY address 0xc
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x3},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulCrossbar,              .u.ul = 11},
  {bp_ulCrossbarPhyId,         .u.ul = 0x1 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},
  {bp_ulPhyId1,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0xbf},
  {bp_ulPhyId0,                .u.ul = (BCM963138_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_0_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_1_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_28_AL},
  {bp_ulPhyId1,                .u.ul = (BCM963138_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_3_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_11_AL},
  {bp_ulPhyId2,                .u.ul = (BCM963138_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_4_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_5_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_12_AL},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 13},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_6_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_7_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_13_AL},
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 12},
  {bp_ulCrossbarPhyId,         .u.ul = 0x18 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},
  {bp_ulPhyId5,                .u.ul = 0x19 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL}, // bottom right.
  {bp_ulPhyId7,                .u.ul = 0x0 |  PHY_INTEGRATED_VALID  | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_128MB| BP_DDR_DEVICE_WIDTH_16},
  {bp_last}
};

static bp_elem_t g_bcm963138dvt_serdes[] = {
  {bp_cpBoardId,               .u.cp = "963138DVT_SEDS"},
  {bp_usSerialLedData,         .u.us = BP_GPIO_0_AH}, // NOTE: bp_ulGpioOverlay is no longer used
  {bp_usSerialLedClk,          .u.us = BP_GPIO_1_AH},
  {bp_usSerialLedMask,         .u.us = BP_GPIO_2_AH},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND}, // Enable NAND pinmux even on SPI boot
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_29_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_30_AL},
  {bp_usGpioLedPwmReserved,    .u.us = BP_GPIO_17_AL}, // pinmux for PWM2 LED
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_15_AL},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_16_AL},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_31_AL},
  {bp_usGpioSecLedAdsl,        .u.us = BP_GPIO_18_AL},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_24_AL}, 
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_20_AL}, // LED register bit 20, shifted serially
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_40 },
  {bp_ulAfeId1,                .u.ul = BP_AFE_CHIP_CH1 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_40 },
  {bp_usGpioIntAFELDPwr,       .u.us = BP_GPIO_52_AH}, // Line Driver 0 = "Int"
  {bp_usGpioIntAFELDData,      .u.us = BP_GPIO_53_AH},
  {bp_usGpioIntAFELDClk,       .u.us = BP_GPIO_55_AH},
  {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_54_AH}, // Line Driver 1 = "Ext"
  {bp_usGpioExtAFELDData,      .u.us = BP_GPIO_9_AH},
  {bp_usGpioExtAFELDClk,       .u.us = BP_GPIO_10_AH},
  {bp_usSpiSlaveSelectNum,     .u.us = 1}, /* define the SPI select for voice */
  {bp_usSpiSlaveSelectGpioNum, .u.us = 127},
  {bp_usSpiSlaveSelectNum,     .u.us = 2},
  {bp_usSpiSlaveSelectGpioNum, .u.us = 25},
  {bp_usSpiSlaveSelectNum,     .u.us = 4},
  {bp_usSpiSlaveSelectGpioNum, .u.us = 8},
  {bp_usGphyBaseAddress,       .u.us = BCM963138_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
                                              // to avoid conflicting P0 and P11 phy address of 1
                                              // the intergrated Quad GPHY address is now 0x8, 0x9, 0xa, 0xb
                                              // the intergrated Single GPHY address 0xc
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x3},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulCrossbar,              .u.ul = 11},
  {bp_ulCrossbarPhyId,         .u.ul = 0x1 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},
  {bp_usSgmiiDetect,           .u.us = BP_GPIO_36_AL},  /* Check the board design if this is valid */
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulPhyId1,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0xbf},
  {bp_ulPhyId0,                .u.ul = (BCM963138_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_0_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_1_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_28_AL},
  {bp_ulPhyId1,                .u.ul = (BCM963138_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_3_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_11_AL},
  {bp_ulPhyId2,                .u.ul = (BCM963138_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_4_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_5_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_12_AL},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 13},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_6_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_7_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_13_AL},
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 12},
  {bp_ulCrossbarPhyId,         .u.ul = 0x18 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},
  {bp_ulPhyId5,                .u.ul = 0x19 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL}, // bottom right.
  {bp_ulPhyId7,                .u.ul = 0x0 |  PHY_INTEGRATED_VALID  | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_128MB| BP_DDR_DEVICE_WIDTH_16},
  {bp_last}
};

static bp_elem_t g_bcm963138dvt_p300[] = {
  {bp_cpBoardId,               .u.cp = "963138DVT_P300"},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_26_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_27_AL},
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x3},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulCrossbar,              .u.ul = 11},
  {bp_ulCrossbarPhyId,         .u.ul = 0x1 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},
  {bp_ulPhyId1,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0xbf},
  {bp_ulPhyId0,                .u.ul = (BCM963138_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_0_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_1_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_28_AL},
  {bp_ulPhyId1,                .u.ul = (BCM963138_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_3_AL},
  {bp_usLinkLed,               .u.us = BP_SERIAL_GPIO_11_AL},
  {bp_ulPhyId2,                .u.ul = (BCM963138_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_5_AL},
  {bp_usLinkLed,               .u.us = BP_SERIAL_GPIO_12_AL},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 13},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  /* BT Uart */
  {bp_usGpioUart2Cts,          .u.ul = BP_GPIO_3_AH}, 
  {bp_usGpioUart2Rts,          .u.ul = BP_GPIO_4_AH},
  {bp_usGpioUart2Sdin,         .u.ul = BP_GPIO_5_AH},
  {bp_usGpioUart2Sdout,        .u.ul = BP_GPIO_6_AH},   
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_6_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_7_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_13_AL},
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 12},
  {bp_ulCrossbarPhyId,         .u.ul = 0x18 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},
  {bp_ulPhyId5,                .u.ul = 0x19 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL}, // bottom right.
  {bp_ulPhyId7,                .u.ul = 0x0 |  PHY_INTEGRATED_VALID  | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963138dvt},
  {bp_last}
};

static bp_elem_t g_bcm963138dvt_p300_serdes[] = {
  {bp_cpBoardId,               .u.cp = "963138DVT_P3SD"},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_26_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_27_AL},
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x3},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulCrossbar,              .u.ul = 11},
  {bp_ulCrossbarPhyId,         .u.ul = 0x1 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},
  {bp_usSgmiiDetect,           .u.us = BP_GPIO_36_AL},  /* Check the board design if this is valid */
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulPhyId1,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0xbf},
  {bp_ulPhyId0,                .u.ul = (BCM963138_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_0_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_1_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_28_AL},
  {bp_ulPhyId1,                .u.ul = (BCM963138_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_3_AL},
  {bp_usLinkLed,               .u.us = BP_SERIAL_GPIO_11_AL},
  {bp_ulPhyId2,                .u.ul = (BCM963138_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_5_AL},
  {bp_usLinkLed,               .u.us = BP_SERIAL_GPIO_12_AL},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 13},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  /* BT Uart */
  {bp_usGpioUart2Cts,          .u.ul = BP_GPIO_3_AH}, 
  {bp_usGpioUart2Rts,          .u.ul = BP_GPIO_4_AH},
  {bp_usGpioUart2Sdin,         .u.ul = BP_GPIO_5_AH},
  {bp_usGpioUart2Sdout,        .u.ul = BP_GPIO_6_AH},   
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_6_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_7_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_13_AL},
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 12},
  {bp_ulCrossbarPhyId,         .u.ul = 0x18 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},
  {bp_ulPhyId5,                .u.ul = 0x19 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL}, // bottom right.
  {bp_ulPhyId7,                .u.ul = 0x0 |  PHY_INTEGRATED_VALID  | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963138dvt},
  {bp_last}
};


static bp_elem_t g_bcm963138ref_bmu[] = {
  {bp_cpBoardId,               .u.cp = "963138REF_BMU"},
  {bp_usBatteryEnable,         .u.us = 1},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND}, // Enable NAND pinmux even on SPI boot
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_22_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_23_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_24_AL}, 
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_17_AL},
  {bp_usGpioSecLedAdsl,        .u.us = BP_GPIO_21_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_CH0 | BP_AFE_LD_6303 | BP_AFE_LD_REV_6303_VR5P3 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_40 }, 
  {bp_ulAfeId1,                .u.ul = BP_AFE_CHIP_CH1 | BP_AFE_LD_6303 | BP_AFE_LD_REV_6303_VR5P3 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_40 }, 
  {bp_usGpioAFEVR5P3PwrEn,     .u.us = BP_GPIO_37_AH},
  {bp_usGpioIntAFELDPwr,       .u.us = BP_GPIO_52_AH}, // Line Driver 0 = "Int"
  {bp_usGpioIntAFELDData,      .u.us = BP_GPIO_53_AH},
  {bp_usGpioIntAFELDClk,       .u.us = BP_GPIO_55_AH},
  {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_54_AH}, // Line Driver 1 = "Ext"
  {bp_usGpioExtAFELDData,      .u.us = BP_GPIO_9_AH},
  {bp_usGpioExtAFELDClk,       .u.us = BP_GPIO_10_AH},
  {bp_usSgmiiDetect,           .u.us = BP_GPIO_28_AL},
  {bp_usSpiSlaveSelectNum,     .u.us = 1}, /* define the SPI select for voice */
  {bp_usSpiSlaveSelectGpioNum, .u.us = 127},
  {bp_usSpiSlaveSelectNum,     .u.us = 2},
  {bp_usSpiSlaveSelectGpioNum, .u.us = 25},
  {bp_usSpiSlaveSelectNum,     .u.us = 4},
  {bp_usSpiSlaveSelectGpioNum, .u.us = 8},
  {bp_usVregSync,              .u.us = BP_GPIO_18_AH},
  {bp_usGphyBaseAddress,       .u.us = BCM963138_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x3},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x4) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulPhyId1,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = (BCM963138_PHY_BASE + 0x0) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_0_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_1_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_28_AL},
  {bp_ulPhyId1,                .u.ul = (BCM963138_PHY_BASE + 0x1) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_2_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_3_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_11_AL},
  {bp_ulPhyId2,                .u.ul = (BCM963138_PHY_BASE + 0x2) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_4_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_5_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_12_AL},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 13},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_6_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_7_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_13_AL},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_GPIO_29_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_GPIO_30_AL},
  {bp_usGpioDectLed,           .u.us = BP_GPIO_31_AL},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_533_8_8_8 | BP_DDR_TOTAL_SIZE_128MB| BP_DDR_DEVICE_WIDTH_16},
  {bp_last}
};

static bp_elem_t g_bcm963138bmu_p202[] = {
  {bp_cpBoardId,               .u.cp = "963138BMU_P202"},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_1024MB| BP_DDR_DEVICE_WIDTH_8},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963138ref_bmu},
  {bp_last}
};

static bp_elem_t g_bcm963138ref_lte[] = {
  {bp_cpBoardId,               .u.cp = "963138REF_LTE"},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND}, // Enable NAND pinmux even on SPI boot
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_29_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_30_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usExtIntrLTE,            .u.us = BP_EXT_INTR_3}, 
  {bp_usGpioLteReset,          .u.us = BP_GPIO_116_AH}, 
  {bp_usGpioUart2Sdin,         .u.us = BP_GPIO_5_AH}, 
  {bp_usGpioUart2Sdout,        .u.us = BP_GPIO_6_AH},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_25_AL}, 
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_31_AL},
  {bp_usGpioSecLedAdsl,        .u.us = BP_GPIO_21_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_40 }, 
  {bp_ulAfeId1,                .u.ul = BP_AFE_CHIP_CH1 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_40 }, 
  {bp_usGpioIntAFELDPwr,       .u.us = BP_GPIO_52_AH}, // Line Driver 0 = "Int"
  {bp_usGpioIntAFELDData,      .u.us = BP_GPIO_53_AH},
  {bp_usGpioIntAFELDClk,       .u.us = BP_GPIO_55_AH},
  {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_54_AH}, // Line Driver 1 = "Ext"
  {bp_usGpioExtAFELDData,      .u.us = BP_GPIO_9_AH},
  {bp_usGpioExtAFELDClk,       .u.us = BP_GPIO_10_AH},
  {bp_usGpioI2cSda,            .u.us = BP_GPIO_117_AH }, /* i2c and sgmii fiber detect selection for serdes interface */
  {bp_usGpioI2cScl,            .u.us = BP_GPIO_119_AH},
  {bp_usSgmiiDetect,           .u.us = BP_GPIO_28_AL},
  {bp_usSpiSlaveSelectNum,     .u.us = 1}, /* define the SPI select for voice */
  {bp_usSpiSlaveSelectGpioNum, .u.us = 127},
  {bp_usGphyBaseAddress,       .u.us = BCM963138_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x3},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulPhyId1,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x2f},
  {bp_ulPhyId0,                .u.ul = (BCM963138_PHY_BASE + 0x0) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_0_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_1_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_28_AL},
  {bp_ulPhyId1,                .u.ul = (BCM963138_PHY_BASE + 0x1) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_2_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_3_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_11_AL},
  {bp_ulPhyId2,                .u.ul = (BCM963138_PHY_BASE + 0x2) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  /*{bp_usSpeedLed100,           .u.us = BP_GPIO_22_AL},*/ //GPIO22 not supported for speed LED in b0
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_23_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_12_AL},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 13},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_24_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_7_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_13_AL},
  {bp_ulPhyId5,                .u.ul = RGMII_DIRECT}, // RGMII1 to MAC port in LTE chip
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_GPIO_18_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_GPIO_17_AL},
  {bp_usTsync1pps,             .u.us = BP_GPIO_4_AL},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_533_8_8_8 | BP_DDR_TOTAL_SIZE_256MB| BP_DDR_DEVICE_WIDTH_16},
  {bp_last}
};

static bp_elem_t g_bcm963138lte_p302[] = {
  {bp_cpBoardId,               .u.cp = "963138LTE_P302"},
  {bp_usSerialLedData,         .u.us = BP_GPIO_0_AH},
  {bp_usSerialLedClk,          .u.us = BP_GPIO_1_AH},
  {bp_usSerialLedMask,         .u.us = BP_GPIO_2_AH},
  {bp_usGpioLteReset,          .u.us = BP_GPIO_3_AH},
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x3},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulPhyId1,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x2f},
  {bp_ulPhyId0,                .u.ul = (BCM963138_PHY_BASE + 0x0) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_0_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_1_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_28_AL},
  {bp_ulPhyId1,                .u.ul = (BCM963138_PHY_BASE + 0x1) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_3_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_11_AL},
  {bp_ulPhyId2,                .u.ul = (BCM963138_PHY_BASE + 0x2) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_5_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_12_AL},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 13},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_24_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_7_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_13_AL},
  {bp_ulPhyId5,                .u.ul = RGMII_DIRECT}, // RGMII1 to MAC port in LTE chip
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963138ref_lte},
  {bp_last}
};

static bp_elem_t g_bcm963138lte2[] = {
  {bp_cpBoardId,               .u.cp = "963138LTE2"},
  {bp_usGpioUart2Cts,          .u.ul = BP_GPIO_20_AH}, 
  {bp_usGpioUart2Rts,          .u.ul = BP_GPIO_21_AH},
  {bp_usGpioSecLedAdsl,        .u.us = BP_SERIAL_GPIO_21_AL},
  {bp_usGpioSecLedAdslFail,    .u.us = BP_SERIAL_GPIO_22_AL},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963138lte_p302},
  {bp_last}
};

/* for B0 chip */
static bp_elem_t g_bcm963138ref_p402[] = {
  {bp_cpBoardId,               .u.cp = "963138REF_P402"},
  {bp_usSerialLedData,         .u.us = BP_GPIO_0_AH}, // NOTE: bp_ulGpioOverlay is no longer used
  {bp_usSerialLedClk,          .u.us = BP_GPIO_1_AH},
  {bp_usSerialLedMask,         .u.us = BP_GPIO_2_AH},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND}, // Enable NAND pinmux even on SPI boot
  {bp_usGpioLedPwmReserved,    .u.us = BP_GPIO_17_AL}, // pinmux for PWM2 LED
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_8_AH},  // placeholder for SF2 Port4 SPD0
  {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_9_AH},  // placeholder for SF2 Port4 SPD1
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_29_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_30_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_24_AL}, 
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_31_AL},
  {bp_usGpioSecLedAdsl,        .u.us = BP_GPIO_27_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_15_AL},
  {bp_usGpioSecLedWanData,     .u.us = BP_GPIO_19_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_40 }, 
  {bp_ulAfeId1,                .u.ul = BP_AFE_CHIP_CH1 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_40 }, 
  {bp_usGpioIntAFELDPwr,       .u.us = BP_GPIO_52_AH}, // Line Driver 0 = "Int"
  {bp_usGpioIntAFELDData,      .u.us = BP_GPIO_53_AH},
  {bp_usGpioIntAFELDClk,       .u.us = BP_GPIO_55_AH},
  {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_54_AH}, // Line Driver 1 = "Ext"
  {bp_usGpioExtAFELDData,      .u.us = BP_GPIO_9_AH},
  {bp_usGpioExtAFELDClk,       .u.us = BP_GPIO_10_AH},
  {bp_usGpioI2cSda,            .u.us = BP_GPIO_117_AH }, /* i2c and sgmii fiber detect selection for serdes interface */
  {bp_usGpioI2cScl,            .u.us = BP_GPIO_119_AH },
  {bp_usSgmiiDetect,           .u.us = BP_GPIO_28_AL},
  {bp_usSpiSlaveSelectNum,     .u.us = 1}, /* define the SPI select for voice */
  {bp_usSpiSlaveSelectGpioNum, .u.us = 127},
  {bp_usSpiSlaveSelectNum,     .u.us = 4},
  {bp_usSpiSlaveSelectGpioNum, .u.us = 8},
  {bp_usSpiSlaveSelectNum,     .u.us = 5},
  {bp_usSpiSlaveSelectGpioNum, .u.us = 7},
  {bp_usGphyBaseAddress,       .u.us = BCM963138_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x3},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  /* use the WAN LED from runner */
  {bp_usSpeedLed100,           .u.us = BP_GPIO_20_AL}, 
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_21_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_22_AL},
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulPhyId1,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = (BCM963138_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_0_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_1_AL},  
  {bp_usLinkLed,               .u.us = BP_SERIAL_GPIO_10_AL},
  {bp_ulPhyId1,                .u.ul = (BCM963138_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_3_AL},  
  {bp_usLinkLed,               .u.us = BP_SERIAL_GPIO_11_AL},
  {bp_ulPhyId2,                .u.ul = (BCM963138_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_5_AL},  
  {bp_usLinkLed,               .u.us = BP_SERIAL_GPIO_12_AL},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 13},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_6_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_7_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_13_AL},
#if 0 /* for RGMII daughter card */
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 12},
  {bp_ulCrossbarPhyId,         .u.ul = 0x18 | PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},  /* make sure the phy id matches the one on the plug in rgmii phy daughter card */
  {bp_ulPhyId5,                .u.ul = 0x19 | PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},  /* make sure the phy id matches the one on the plug in rgmii phy daughter card */
#endif
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_25_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_26_AL},
  {bp_usGpioPotsLed,           .u.us = BP_SERIAL_GPIO_28_AL},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_533_8_8_8 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16},
  {bp_last}
};


static bp_elem_t g_bcm963138ref_gfast[] = {
  {bp_cpBoardId,               .u.cp = "963138_GFAST"},
  {bp_usSerialLedData,         .u.us = BP_GPIO_0_AH}, // NOTE: bp_ulGpioOverlay is no longer used
  {bp_usSerialLedClk,          .u.us = BP_GPIO_1_AH},
  {bp_usSerialLedMask,         .u.us = BP_GPIO_2_AH},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND}, // Enable NAND pinmux even on SPI boot
  {bp_usGpioLedPwmReserved,    .u.us = BP_GPIO_17_AL}, // pinmux for PWM2 LED
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_29_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_30_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_24_AL}, 
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_31_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_15_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_GFAST | BP_AFE_LD_6303 | BP_AFE_FE_REV_6303_REV_12_3_40 }, 
  {bp_ulAfeId1,                .u.ul = BP_AFE_CHIP_CH1 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_40 }, 
  {bp_usGpioIntAFELDPwr,       .u.us = BP_GPIO_52_AH}, // Line Driver 0 = "Int"
  {bp_usGpioIntAFELDData,      .u.us = BP_GPIO_53_AH},
  {bp_usGpioIntAFELDClk,       .u.us = BP_GPIO_55_AH},
  {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_54_AH}, // Line Driver 1 = "Ext"
  {bp_usGpioExtAFELDData,      .u.us = BP_GPIO_9_AH},
  {bp_usGpioExtAFELDClk,       .u.us = BP_GPIO_10_AH},
  {bp_usGphyBaseAddress,       .u.us = BCM963138_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x3},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  /* use the WAN LED from runner */
  {bp_ulPhyId1,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = (BCM963138_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_0_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_1_AH},  
  {bp_usLinkLed,               .u.us = BP_GPIO_28_AL},
  {bp_ulPhyId1,                .u.ul = (BCM963138_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_2_AH},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_3_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_11_AL},
  {bp_ulPhyId2,                .u.ul = (BCM963138_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_4_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_5_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_12_AL},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 13},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_6_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_7_AH},  
  {bp_usLinkLed,               .u.us = BP_GPIO_13_AL},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_128MB| BP_DDR_DEVICE_WIDTH_16},
  {bp_last}
};


static bp_elem_t g_bcm963138bg_gfast[] = {
  {bp_cpBoardId,               .u.cp = "963138BG_GFAST"},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_24_AL},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_28_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_GFAST | BP_AFE_LD_6303 | BP_AFE_FE_REV_6303_REV_12_3_40 },
  {bp_usGpioIntAFELDPwr,       .u.us = BP_GPIO_52_AH}, // Line Driver 0 = "Int"
  {bp_usGphyBaseAddress,       .u.us = BCM963138_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x2},
  {bp_ulPhyId1,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x10},
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usLinkLed,               .u.us = BP_GPIO_14_AL},
  {bp_usVregSync,              .u.us = BP_GPIO_37_AH},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_533_8_8_8 | BP_DDR_TOTAL_SIZE_128MB| BP_DDR_DEVICE_WIDTH_16},
  {bp_last}
};

static bp_elem_t g_bcm963138ref_gfast_p40x[] = {
  {bp_cpBoardId,               .u.cp = "963138GFP40X"},
  {bp_usSpiSlaveSelectNum,     .u.us = 4},
  {bp_usSpiSlaveSelectGpioNum, .u.us = 8},
  {bp_usSpiSlaveSelectNum,     .u.us = 5},
  {bp_usSpiSlaveSelectGpioNum, .u.us = 7},
  {bp_usGphyBaseAddress,       .u.us = BCM963138_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x3},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
 /* use the WAN LED from runner */
  {bp_ulPhyId1,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = (BCM963138_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_0_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_1_AH},  
  {bp_usLinkLed,               .u.us = BP_GPIO_28_AL},
  {bp_ulPhyId1,                .u.ul = (BCM963138_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_2_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_3_AH},  
  {bp_usLinkLed,               .u.us = BP_GPIO_11_AL},
  {bp_ulPhyId2,                .u.ul = (BCM963138_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_4_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_5_AH},  
  {bp_usLinkLed,               .u.us = BP_GPIO_12_AL},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 13},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_6_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_7_AH},  
  {bp_usLinkLed,               .u.us = BP_GPIO_13_AL},
  {bp_usGpioAFELDRelay,	       .u.us = BP_GPIO_3_AH},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963138ref_gfast},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_last}
};

static bp_elem_t g_bcm963138ref_gfast_p40x_mem[] = {
  {bp_cpBoardId,               .u.cp = "138GFAST4G"},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963138ref_gfast_p40x},
  {bp_last}
};

static bp_elem_t g_bcm963138ref_gfast_2[] = {
  {bp_cpBoardId,               .u.cp = "963138GFAST2"},
  {bp_usSerialLedData,         .u.us = BP_GPIO_0_AH}, // NOTE: bp_ulGpioOverlay is no longer used
  {bp_usSerialLedClk,          .u.us = BP_GPIO_1_AH},
  {bp_usSerialLedMask,         .u.us = BP_GPIO_2_AH},
  {bp_usGpioLedWanData,        .u.us = BP_SERIAL_GPIO_15_AH},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_16_AH},
  {bp_usGpioSecLedWanData,     .u.us = BP_SERIAL_GPIO_19_AH},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_18_AH},
  {bp_usGpioSecLedAdsl,        .u.us = BP_SERIAL_GPIO_21_AH},
  {bp_usGpioLedReserved,    .u.us = BP_SERIAL_GPIO_20_AH},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_17_AH},
  {bp_usSpiSlaveSelectNum,     .u.us = 4},
  {bp_usSpiSlaveSelectGpioNum, .u.us = 8},
  {bp_usSpiSlaveSelectNum,     .u.us = 5},
  {bp_usSpiSlaveSelectGpioNum, .u.us = 7},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_REV_6303_REV_12_3_40 },
  {bp_ulAfeId1,                .u.ul = BP_AFE_CHIP_CH1 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_40 },
  {bp_usGpioIntAFELDPwr,       .u.us = BP_GPIO_52_AH}, // Line Driver 0 = "Int"
  {bp_usGpioIntAFELDData,      .u.us = BP_GPIO_53_AH},
  {bp_usGpioIntAFELDClk,       .u.us = BP_GPIO_55_AH},
  {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_54_AH}, // Line Driver 1 = "Ext"
  {bp_usGpioExtAFELDData,      .u.us = BP_GPIO_9_AH},
  {bp_usGpioExtAFELDClk,       .u.us = BP_GPIO_10_AH},
  {bp_usGphyBaseAddress,       .u.us = BCM963138_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x3},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_8_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_9_AH},
  {bp_usLinkLed,               .u.us = BP_SERIAL_GPIO_14_AH},
  {bp_ulPhyId1,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = (BCM963138_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_100_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_101_AL},
  {bp_usLinkLed,               .u.us = BP_SERIAL_GPIO_10_AH},
  {bp_ulPhyId1,                .u.ul = (BCM963138_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_102_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_103_AL},
  {bp_usLinkLed,               .u.us = BP_SERIAL_GPIO_11_AH},
  {bp_ulPhyId2,                .u.ul = (BCM963138_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_104_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_105_AL},
  {bp_usLinkLed,               .u.us = BP_SERIAL_GPIO_12_AH},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 13},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_6_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_7_AH},
  {bp_usLinkLed,               .u.us = BP_SERIAL_GPIO_13_AH},
  {bp_usGpioAFELDRelay,	       .u.us = BP_GPIO_3_AH},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_116_AL | BP_LED_USE_GPIO },
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_118_AL | BP_LED_USE_GPIO },
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_usVregSync,              .u.us = BP_GPIO_37_AH},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16},
  {bp_last}
};


static bp_elem_t g_bcm963138ref_gfast_4[] = {
  {bp_cpBoardId,               .u.cp = "963138GFAST4"},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963138ref_gfast_2},
};

/* for B0 chip */
static bp_elem_t g_bcm963138ref_p502[] = {
  {bp_cpBoardId,               .u.cp = "963138REF_P502"},
  {bp_usSgmiiDetect,           .u.us = BP_GPIO_28_AL},
  {bp_usGphyBaseAddress,       .u.us = BCM963138_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x3},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  /* use the WAN LED from runner */
  {bp_usSpeedLed100,           .u.us = BP_GPIO_20_AL}, 
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_21_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_22_AL},
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulPhyId1,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = (BCM963138_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_0_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_1_AH},  
  {bp_usLinkLed,               .u.us = BP_SERIAL_GPIO_10_AH},
  {bp_ulPhyId1,                .u.ul = (BCM963138_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_2_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_3_AH},  
  {bp_usLinkLed,               .u.us = BP_SERIAL_GPIO_11_AH},
  {bp_ulPhyId2,                .u.ul = (BCM963138_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_4_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_5_AH},  
  {bp_usLinkLed,               .u.us = BP_SERIAL_GPIO_12_AH},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 13},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  /* BT Uart */
  {bp_usGpioUart2Cts,          .u.ul = BP_GPIO_3_AH},
  {bp_usGpioUart2Rts,          .u.ul = BP_GPIO_4_AH},
  {bp_usGpioUart2Sdin,         .u.ul = BP_GPIO_5_AH},
  {bp_usGpioUart2Sdout,        .u.ul = BP_GPIO_6_AH},
  {bp_usGpioBtWake,            .u.us = BP_GPIO_116_AH},
  {bp_usGpioBtReset,           .u.us = BP_GPIO_118_AH},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_6_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_7_AH},  
  {bp_usLinkLed,               .u.us = BP_GPIO_13_AL},
#if 0 /* for RGMII daughter card */
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 12},
  {bp_ulCrossbarPhyId,         .u.ul = 0x18 | PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},  /* make sure the phy id matches the one on the plug in rgmii phy daughter card */
  {bp_ulPhyId5,                .u.ul = 0x19 | PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},  /* make sure the phy id matches the one on the plug in rgmii phy daughter card */
#endif
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_25_AH},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_26_AH},
  {bp_usGpioPotsLed,           .u.us = BP_SERIAL_GPIO_28_AH},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963138ref_p402},
  {bp_last}
};

static bp_elem_t g_bcm963138ref_p802[] = {
  {bp_cpBoardId,               .u.cp = "963138REF_P802"},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_IRQ | BP_EXT_INTR_5},
  {bp_usGpioSfpDetect,         .u.us = BP_GPIO_109_AL},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963138ref_p502},
  {bp_last}
};

static bp_elem_t g_bcm963138ref_p502_plc[] = {
  {bp_cpBoardId,               .u.cp = "963138P502_PLC"},
  {bp_usGpioI2cSda,            .u.us = BP_GPIO_117_AH }, /* i2c and sgmii fiber detect selection for serdes interface */
  {bp_usGpioI2cScl,            .u.us = BP_GPIO_119_AH },
  {bp_usSgmiiDetect,           .u.us = BP_GPIO_28_AL},
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x3},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  /* use the WAN LED from runner */
  {bp_usSpeedLed100,           .u.us = BP_GPIO_20_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_21_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_22_AL},
  {bp_ulCrossbar,              .u.ul = 9},
  {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulPhyId1,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = (BCM963138_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_0_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_1_AH},  
  {bp_usLinkLed,               .u.us = BP_SERIAL_GPIO_10_AH},
  {bp_ulPhyId1,                .u.ul = (BCM963138_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_2_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_3_AH},  
  {bp_usLinkLed,               .u.us = BP_SERIAL_GPIO_11_AH},
  {bp_ulPhyId2,                .u.ul = (BCM963138_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_4_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_5_AH},  
  {bp_usLinkLed,               .u.us = BP_SERIAL_GPIO_12_AH},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 13},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_6_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_7_AH},  
  {bp_usLinkLed,               .u.us = BP_GPIO_13_AL},
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_usPhyConnType,           .u.us = PHY_CONN_TYPE_PLC},
  {bp_ucPhyDevName,            .u.cp = "plc%d"},
  {bp_ulCrossbar,              .u.ul = 12},
  {bp_ulCrossbarPhyId,         .u.ul = RGMII_DIRECT_3P3V },
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_TX_INTERNAL_DELAY | PORT_FLAG_RX_INTERNAL_DELAY},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963138ref_p502},
  {bp_last}
};

static bp_elem_t g_bcm963138ref_p502_moca[] = {
  {bp_cpBoardId,               .u.cp = "963138P502_MOCA"},
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x3},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  /* use the WAN LED from runner */
  {bp_usSpeedLed100,           .u.us = BP_GPIO_20_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_21_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_22_AL},
  {bp_ulPhyId1,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x8f},
  {bp_ulPhyId0,                .u.ul = (BCM963138_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_0_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_1_AH},  
  {bp_usLinkLed,               .u.us = BP_SERIAL_GPIO_10_AH},
  {bp_ulPhyId1,                .u.ul = (BCM963138_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_2_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_3_AH},  
  {bp_usLinkLed,               .u.us = BP_SERIAL_GPIO_11_AH},
  {bp_ulPhyId2,                .u.ul = (BCM963138_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_4_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_5_AH},  
  {bp_usLinkLed,               .u.us = BP_SERIAL_GPIO_12_AH},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 13},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_6_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_7_AH},  
  {bp_usLinkLed,               .u.us = BP_GPIO_13_AL},
  {bp_ulPhyId7,                .u.ul = RGMII_DIRECT | MAC_IF_RGMII_2P5V },
  {bp_usPhyConnType,           .u.us = PHY_CONN_TYPE_MOCA},
  {bp_ucPhyDevName,            .u.cp = "moca%d"},
  {bp_usMocaType0,             .u.us = BP_MOCA_TYPE_LAN},
  {bp_usMocaRfBand,            .u.us = BP_MOCA_RF_BAND_D_LOW},
  {bp_usExtIntrMocaHostIntr,   .u.us = BP_EXT_INTR_TYPE_IRQ_HIGH_LEVEL | BP_EXT_INTR_4},
#if !defined(_CFE_)
  {bp_pMocaInit,               .u.ptr = (void*)moca6802InitSeq},
#endif
  {bp_usGpioSpiSlaveReset,     .u.us = BP_GPIO_110_AL},
  {bp_usGpioSpiSlaveBootMode,  .u.us = BP_GPIO_14_AL},
  {bp_usSpiSlaveBusNum,        .u.us = HS_SPI_BUS_NUM},
  {bp_usSpiSlaveSelectNum,     .u.us = 3},
  {bp_usSpiSlaveSelectGpioNum, .u.us = 26},
  {bp_usSpiSlaveMode,          .u.us = SPI_MODE_3},
  {bp_ulSpiSlaveCtrlState,     .u.ul = SPI_CONTROLLER_STATE_GATE_CLK_SSOFF},
  {bp_ulSpiSlaveMaxFreq,       .u.ul = 12500000},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963138ref_p502},
  {bp_last}
};

static bp_elem_t g_bcm963138ref_p502_bhr[] = {
  {bp_cpBoardId,               .u.cp = "963138P502_BHR"},
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x3},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ucPhyDevName,            .u.cp = "moca%d"},
  {bp_ulCrossbar,              .u.ul = 12},
  {bp_ulCrossbarPhyId,         .u.ul = RGMII_DIRECT | MAC_IF_RGMII_2P5V },
  {bp_usPhyConnType,           .u.us = PHY_CONN_TYPE_MOCA},
  /* use the WAN LED from runner */
  {bp_usSpeedLed100,           .u.us = BP_GPIO_20_AL}, 
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_21_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_22_AL},
  {bp_ulPhyId1,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x8f},
  {bp_ulPhyId0,                .u.ul = (BCM963138_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_0_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_1_AH},  
  {bp_usLinkLed,               .u.us = BP_SERIAL_GPIO_10_AH},
  {bp_ulPhyId1,                .u.ul = (BCM963138_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_2_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_3_AH},  
  {bp_usLinkLed,               .u.us = BP_SERIAL_GPIO_11_AH},
  {bp_ulPhyId2,                .u.ul = (BCM963138_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_4_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_5_AH},  
  {bp_usLinkLed,               .u.us = BP_SERIAL_GPIO_12_AH},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 13},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_6_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_7_AH},  
  {bp_usLinkLed,               .u.us = BP_GPIO_13_AL},
  {bp_ulPhyId7,                .u.ul = RGMII_DIRECT | MAC_IF_RGMII_2P5V },
  {bp_usPhyConnType,           .u.us = PHY_CONN_TYPE_MOCA},
  {bp_ucPhyDevName,            .u.cp = "moca%d"},
  {bp_usMocaType0,             .u.us = BP_MOCA_TYPE_WAN},
  {bp_usMocaRfBand,            .u.us = BP_MOCA_RF_BAND_D_HIGH},
  {bp_usExtIntrMocaHostIntr,   .u.us = BP_EXT_INTR_TYPE_IRQ_HIGH_LEVEL | BP_EXT_INTR_2},
#if !defined(_CFE_)
  {bp_pMocaInit,               .u.ptr = (void*)moca6802InitSeq},
#endif
  {bp_usGpioSpiSlaveReset,     .u.us = BP_GPIO_115_AL},
  {bp_usGpioSpiSlaveBootMode,  .u.us = BP_GPIO_60_AL},
  {bp_usSpiSlaveBusNum,        .u.us = HS_SPI_BUS_NUM},
  {bp_usSpiSlaveSelectNum,     .u.us = 2},
  {bp_usSpiSlaveSelectGpioNum, .u.us = 25},
  {bp_usSpiSlaveMode,          .u.us = SPI_MODE_3},
  {bp_ulSpiSlaveCtrlState,     .u.ul = SPI_CONTROLLER_STATE_GATE_CLK_SSOFF},
  {bp_ulSpiSlaveMaxFreq,       .u.ul = 12500000},
  {bp_usMocaType1,             .u.us = BP_MOCA_TYPE_LAN},
  {bp_usMocaRfBand,            .u.us = BP_MOCA_RF_BAND_D_LOW},
  {bp_usExtIntrMocaHostIntr,   .u.us = BP_EXT_INTR_TYPE_IRQ_HIGH_LEVEL | BP_EXT_INTR_4},
#if !defined(_CFE_)
  {bp_pMocaInit,               .u.ptr = (void*)moca6802InitSeq},
#endif
  {bp_usGpioSpiSlaveReset,     .u.us = BP_GPIO_110_AL},
  {bp_usGpioSpiSlaveBootMode,  .u.us = BP_GPIO_14_AL},
  {bp_usSpiSlaveBusNum,        .u.us = HS_SPI_BUS_NUM},
  {bp_usSpiSlaveSelectNum,     .u.us = 3},
  {bp_usSpiSlaveSelectGpioNum, .u.us = 26},
  {bp_usSpiSlaveMode,          .u.us = SPI_MODE_3},
  {bp_ulSpiSlaveCtrlState,     .u.ul = SPI_CONTROLLER_STATE_GATE_CLK_SSOFF},
  {bp_ulSpiSlaveMaxFreq,       .u.ul = 12500000},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963138ref_p502},
  {bp_last}
};

static bp_elem_t g_bcm963138ref_rnc[] = {
  {bp_cpBoardId,               .u.cp = "963138REF_RNC"},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND}, // Enable NAND pinmux even on SPI boot
  {bp_usSerialLedData,         .u.us = BP_GPIO_0_AH},
  {bp_usSerialLedClk,          .u.us = BP_GPIO_1_AH},
  {bp_usSerialLedMask,         .u.us = BP_GPIO_2_AH},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_29_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_30_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usExtIntrLTE,            .u.us = BP_EXT_INTR_3}, 
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_24_AL},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_31_AL},
  {bp_usGpioSecLedAdsl,        .u.us = BP_GPIO_18_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_15_AL},
  {bp_usGpioSecLedWanData,     .u.us = BP_GPIO_19_AL},
  {bp_usGpioLteReset,          .u.us = BP_GPIO_110_AH},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_40 }, 
  {bp_ulAfeId1,                .u.ul = BP_AFE_CHIP_CH1 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_40 }, 
  {bp_usGpioIntAFELDPwr,       .u.us = BP_GPIO_52_AH}, // Line Driver 0 = "Int"
  {bp_usGpioIntAFELDData,      .u.us = BP_GPIO_53_AH},
  {bp_usGpioIntAFELDClk,       .u.us = BP_GPIO_55_AH},
  {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_54_AH}, // Line Driver 1 = "Ext"
  {bp_usGpioExtAFELDData,      .u.us = BP_GPIO_9_AH},
  {bp_usGpioExtAFELDClk,       .u.us = BP_GPIO_10_AH},
  {bp_usSpiSlaveSelectNum,     .u.us = 1}, /* define the SPI select for voice */
  {bp_usSpiSlaveSelectGpioNum, .u.us = 127},
  {bp_usVregSync,              .u.us = BP_GPIO_37_AH},
  {bp_usGphyBaseAddress,       .u.us = BCM963138_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x3},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_20_AL}, /* use the WAN LED from runner */
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_21_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_22_AL},
  {bp_ulPhyId1,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = (BCM963138_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_0_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_1_AH},
  {bp_usLinkLed,               .u.us = BP_GPIO_28_AH},
  {bp_ulPhyId1,                .u.ul = (BCM963138_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_2_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_3_AH},
  {bp_usLinkLed,               .u.us = BP_GPIO_11_AH},
  {bp_ulPhyId2,                .u.ul = (BCM963138_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_4_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_5_AH},
  {bp_usLinkLed,               .u.us = BP_GPIO_12_AH},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 13},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_6_AH},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_7_AH},
  {bp_usLinkLed,               .u.us = BP_GPIO_13_AL},
  /* BT Uart */
  {bp_usGpioUart2Cts,          .u.ul = BP_GPIO_3_AH},
  {bp_usGpioUart2Rts,          .u.ul = BP_GPIO_4_AH},
  {bp_usGpioUart2Sdin,         .u.ul = BP_GPIO_5_AH},
  {bp_usGpioUart2Sdout,        .u.ul = BP_GPIO_6_AH},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16},
  {bp_last}
};

static bp_elem_t g_bcm963138ref_rncp400[] = {
  {bp_cpBoardId,               .u.cp = "138REFrncP400"},
  {bp_usGpioReserved,          .u.us = BP_GPIO_17_AL},
  {bp_usGpioReserved,          .u.us = BP_GPIO_110_AL},
  {bp_usGpioBtWake,            .u.us = BP_GPIO_116_AH},
  {bp_usGpioBtReset,           .u.us = BP_GPIO_118_AH},
  {bp_usNtrRefIn,              .u.us = BP_GPIO_25_AL},
  {bp_usGpioI2cSda,            .u.us = BP_GPIO_117_AH},
  {bp_usGpioI2cScl,            .u.us = BP_GPIO_119_AH},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963138ref_rnc},
  {bp_last}
};

static bp_elem_t g_bcm963138ref_bgw[] = {
  {bp_cpBoardId,               .u.cp = "963138REF_BGW"},
  {bp_usVregSync,              .u.us = BP_GPIO_18_AH},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND}, // Enable NAND pinmux even on SPI boot
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usExtIntrLTE,            .u.us = BP_EXT_INTR_3}, 
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_24_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_15_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_40 },
  {bp_ulAfeId1,                .u.ul = BP_AFE_CHIP_CH1 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_40 },
  {bp_usGpioIntAFELDPwr,       .u.us = BP_GPIO_52_AH}, // Line Driver 0 = "Int"
  {bp_usGpioIntAFELDData,      .u.us = BP_GPIO_53_AH},
  {bp_usGpioIntAFELDClk,       .u.us = BP_GPIO_55_AH},
  {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_54_AH}, // Line Driver 1 = "Ext"
  {bp_usGpioExtAFELDData,      .u.us = BP_GPIO_9_AH},
  {bp_usGpioExtAFELDClk,       .u.us = BP_GPIO_10_AH},
  {bp_usSpiSlaveSelectNum,     .u.us = 1}, /* define the SPI select for voice */
  {bp_usSpiSlaveSelectGpioNum, .u.us = 127},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGphyBaseAddress,       .u.us = BCM963138_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x3},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_20_AL}, /* use the WAN LED from runner */
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_21_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_22_AL},
  {bp_ulPhyId1,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = (BCM963138_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_0_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_1_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_28_AL},
  {bp_ulPhyId1,                .u.ul = (BCM963138_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_2_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_3_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_11_AL},
  {bp_ulPhyId2,                .u.ul = (BCM963138_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_4_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_5_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_12_AL},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 13},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_6_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_7_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_13_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_26_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_23_AL},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_17_AL},
  {bp_usGpioSecLedAdsl,        .u.us = BP_GPIO_27_AL},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_16_AL},
  {bp_usGpioVoip1Led,          .u.us = BP_GPIO_29_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_GPIO_30_AL},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16},
  {bp_last}
};

static bp_elem_t g_bcm963138rref_gfast[] = {
  {bp_cpBoardId,               .u.cp = "63138RREF_GFAST"},
  {bp_usGpioAFELDRelay,        .u.us = BP_GPIO_19_AL},
  {bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_25_AH},
  {bp_usVregSync,              .u.us = BP_GPIO_18_AH},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_26_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_23_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXC | BP_AFE_FE_REV_6303_REV_12_3_40  | BP_AFE_FE_8dBm},
  {bp_ulAfeId1,                .u.ul = BP_AFE_CHIP_CH1 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXC | BP_AFE_FE_REV_6303_REV_12_3_40},
  {bp_usGpioIntAFELDPwr,       .u.us = BP_GPIO_52_AH}, // Line Driver 0 = "Int"
  {bp_usGpioIntAFELDData,      .u.us = BP_GPIO_53_AH},
  {bp_usGpioIntAFELDClk,       .u.us = BP_GPIO_55_AH},
  {bp_usGphyBaseAddress,       .u.us = BCM963138_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x3},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_20_AL}, /* use the WAN LED from runner */
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_21_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_22_AL},
  {bp_usSpiSlaveSelectNum,     .u.us = 1}, /* define the SPI select for voice */
  {bp_usSpiSlaveSelectGpioNum, .u.us = 127},
  /* use the WAN LED from runner */
  {bp_ulPhyId1,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = (BCM963138_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_0_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_1_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_10_AL},
  {bp_ulPhyId1,                .u.ul = (BCM963138_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_2_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_3_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_11_AL},
  {bp_ulPhyId2,                .u.ul = (BCM963138_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_4_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_5_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_12_AL},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 13},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_6_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_7_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_13_AL},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_17_AL},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_16_AL},
  {bp_usGpioVoip1Led,          .u.us = BP_GPIO_29_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_GPIO_30_AL},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_15_AL},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_1024MB| BP_DDR_DEVICE_WIDTH_8},
  {bp_last}
};

static bp_elem_t g_bcm963138rref_gfast_rgmii[] = {
  {bp_cpBoardId,               .u.cp = "38R_GFAST_RGMII"},
  {bp_usGphyBaseAddress,       .u.us = BCM963138_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x3},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_20_AL}, /* use the WAN LED from runner */
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_21_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_22_AL},
  /* use the WAN LED from runner */
  {bp_ulPhyId1,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0xaf},
  {bp_ulPhyId0,                .u.ul = (BCM963138_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_0_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_1_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_10_AL},
  {bp_ulPhyId1,                .u.ul = (BCM963138_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_2_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_3_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_11_AL},
  {bp_ulPhyId2,                .u.ul = (BCM963138_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_4_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_5_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_12_AL},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 13},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_6_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_7_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_13_AL},
  {bp_ulPhyId5,                .u.ul = 0x18 | PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},  /* make sure the phy id matches the one on the plug in rgmii phy daughter card */
  {bp_ulPhyId7,                .u.ul = 0x19 | PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},  /* make sure the phy id matches the one on the plug in rgmii phy daughter card */
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963138rref_gfast},
  {bp_last}
};

static bp_elem_t g_bcm963138rref_rnc[] = {
  {bp_cpBoardId,               .u.cp = "63138RREF_RNC"},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_40|BP_AFE_FE_RNC },
  {bp_ulAfeId1,                .u.ul = BP_AFE_CHIP_CH1 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_40|BP_AFE_FE_RNC },
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963138rref_gfast},
  {bp_last}
};

static bp_elem_t g_bcm963138rref_rnc_rgmii[] = {
  {bp_cpBoardId,               .u.cp = "138R_RNC_RGMII"},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_40|BP_AFE_FE_RNC },
  {bp_ulAfeId1,                .u.ul = BP_AFE_CHIP_CH1 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_40|BP_AFE_FE_RNC },
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963138rref_gfast_rgmii},
  {bp_last}
};

static bp_elem_t g_bcm963138rref_7lan[] = {                 // special profile for QA to test all 7 lan ports 
  {bp_cpBoardId,               .u.cp = "63138RREF_7LAN"},
  {bp_usGphyBaseAddress,       .u.us = BCM963138_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x2},            // no WAN port 
  {bp_usSpeedLed100,           .u.us = BP_GPIO_20_AL}, /* use the WAN LED from runner */
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_21_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_22_AL},
  /* use the WAN LED from runner */
  {bp_ulPhyId1,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0xbf},
  {bp_ulPhyId0,                .u.ul = (BCM963138_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ucPhyDevName,             .u.cp = "eth1"},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_0_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_1_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_10_AL},
  {bp_ulPhyId1,                .u.ul = (BCM963138_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ucPhyDevName,             .u.cp = "eth2"},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_2_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_3_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_11_AL},
  {bp_ulPhyId2,                .u.ul = (BCM963138_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ucPhyDevName,             .u.cp = "eth3"},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_4_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_5_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_12_AL},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ucPhyDevName,             .u.cp = "eth4"},
  {bp_ulCrossbar,              .u.ul = 13},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_6_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_7_AL},
  {bp_usLinkLed,               .u.us = BP_GPIO_13_AL},
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ucPhyDevName,             .u.cp = "eth5"},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId5,                .u.ul = 0x18 | PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},  /* make sure the phy id matches the one on the plug in rgmii phy daughter card */
  {bp_ucPhyDevName,             .u.cp = "eth6"},
  {bp_ulPhyId7,                .u.ul = 0x19 | PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},  /* make sure the phy id matches the one on the plug in rgmii phy daughter card */
  {bp_ucPhyDevName,             .u.cp = "eth7"},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963138rref_gfast},
  {bp_last}
};

/* single moca (lan) using p200 daughter card */
static bp_elem_t g_bcm963138ref_p502_moca_p200[] = {
  {bp_cpBoardId,               .u.cp = "963138P502_M200"},
  {bp_usMocaType0,             .u.us = BP_MOCA_TYPE_LAN},
  {bp_usMocaRfBand,            .u.us = BP_MOCA_RF_BAND_D_LOW},
  {bp_usExtIntrMocaHostIntr,   .u.us = BP_EXT_INTR_TYPE_IRQ_HIGH_LEVEL | BP_EXT_INTR_4},
#if !defined(_CFE_)
  {bp_pMocaInit,               .u.ptr = (void*)moca6802InitSeq},
#endif
  {bp_usGpioSpiSlaveReset,     .u.us = BP_GPIO_110_AL},
  {bp_usGpioSpiSlaveBootMode,  .u.us = BP_GPIO_13_AL},
  {bp_usSpiSlaveBusNum,        .u.us = HS_SPI_BUS_NUM},
  {bp_usSpiSlaveSelectNum,     .u.us = 3},
  {bp_usSpiSlaveSelectGpioNum, .u.us = 26},
  {bp_usSpiSlaveMode,          .u.us = SPI_MODE_3},
  {bp_ulSpiSlaveCtrlState,     .u.ul = SPI_CONTROLLER_STATE_GATE_CLK_SSOFF},
  {bp_ulSpiSlaveMaxFreq,       .u.ul = 12500000},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963138ref_p502_moca},
  {bp_last}
};

/* dual moca using p200 daughter cards */
static bp_elem_t g_bcm963138ref_p502_dual_moca_p200[] = {
  {bp_cpBoardId,               .u.cp = "138P502_DM200"},
  {bp_usMocaType0,             .u.us = BP_MOCA_TYPE_WAN},
  {bp_usMocaRfBand,            .u.us = BP_MOCA_RF_BAND_D_HIGH},
  {bp_usExtIntrMocaHostIntr,   .u.us = BP_EXT_INTR_TYPE_IRQ_HIGH_LEVEL | BP_EXT_INTR_2},
#if !defined(_CFE_)
  {bp_pMocaInit,               .u.ptr = (void*)moca6802InitSeq},
#endif
  {bp_usGpioSpiSlaveReset,     .u.us = BP_GPIO_115_AL},
  {bp_usGpioSpiSlaveBootMode,  .u.us = BP_GPIO_61_AL},
  {bp_usSpiSlaveBusNum,        .u.us = HS_SPI_BUS_NUM},
  {bp_usSpiSlaveSelectNum,     .u.us = 2},
  {bp_usSpiSlaveSelectGpioNum, .u.us = 25},
  {bp_usSpiSlaveMode,          .u.us = SPI_MODE_3},
  {bp_ulSpiSlaveCtrlState,     .u.ul = SPI_CONTROLLER_STATE_GATE_CLK_SSOFF},
  {bp_ulSpiSlaveMaxFreq,       .u.ul = 12500000},
  {bp_usMocaType1,             .u.us = BP_MOCA_TYPE_LAN},
  {bp_usMocaRfBand,            .u.us = BP_MOCA_RF_BAND_D_LOW},
  {bp_usExtIntrMocaHostIntr,   .u.us = BP_EXT_INTR_TYPE_IRQ_HIGH_LEVEL | BP_EXT_INTR_4},
#if !defined(_CFE_)
  {bp_pMocaInit,               .u.ptr = (void*)moca6802InitSeq},
#endif
  {bp_usGpioSpiSlaveReset,     .u.us = BP_GPIO_110_AL},
  {bp_usGpioSpiSlaveBootMode,  .u.us = BP_GPIO_13_AL},
  {bp_usSpiSlaveBusNum,        .u.us = HS_SPI_BUS_NUM},
  {bp_usSpiSlaveSelectNum,     .u.us = 3},
  {bp_usSpiSlaveSelectGpioNum, .u.us = 26},
  {bp_usSpiSlaveMode,          .u.us = SPI_MODE_3},
  {bp_ulSpiSlaveCtrlState,     .u.ul = SPI_CONTROLLER_STATE_GATE_CLK_SSOFF},
  {bp_ulSpiSlaveMaxFreq,       .u.ul = 12500000},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963138ref_p502_bhr},
  {bp_last}
};

static bp_elem_t g_bcm963138ref_3wlan[] = {
  {bp_cpBoardId,               .u.cp = "138REF_3WLAN"},
  {bp_usSerialLedData,         .u.us = BP_GPIO_0_AH}, // NOTE: bp_ulGpioOverlay is no longer used
  {bp_usSerialLedClk,          .u.us = BP_GPIO_1_AH},
  {bp_usSerialLedMask,         .u.us = BP_GPIO_2_AH},
  {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND}, // Enable NAND pinmux even on SPI boot
  // {bp_usGpioLedReserved,       .u.us = BP_GPIO_17_AL}, // pinmux for PWM2 LED
  // {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_8_AH},  // placeholder for SF2 Port4 SPD0
  // {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_9_AH},  // placeholder for SF2 Port4 SPD1
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_29_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_30_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_24_AL}, 
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_31_AL},
  {bp_usGpioSecLedAdsl,        .u.us = BP_GPIO_25_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_15_AL},
  {bp_usGpioSecLedWanData,     .u.us = BP_GPIO_19_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_40 }, 
  {bp_ulAfeId1,                .u.ul = BP_AFE_CHIP_CH1 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_REV_12_3_40 }, 
  {bp_usGpioIntAFELDPwr,       .u.us = BP_GPIO_52_AH}, // Line Driver 0 = "Int"
  {bp_usGpioIntAFELDData,      .u.us = BP_GPIO_53_AH},
  {bp_usGpioIntAFELDClk,       .u.us = BP_GPIO_55_AH},
  {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_54_AH}, // Line Driver 1 = "Ext"
  {bp_usGpioExtAFELDData,      .u.us = BP_GPIO_9_AH},
  {bp_usGpioExtAFELDClk,       .u.us = BP_GPIO_10_AH},
  {bp_usGpioI2cSda,            .u.us = BP_GPIO_117_AH }, /* i2c and sgmii fiber detect selection for serdes interface */
  {bp_usGpioI2cScl,            .u.us = BP_GPIO_119_AH },
  {bp_usGphyBaseAddress,       .u.us = BCM963138_PHY_BASE},  // use phy addressses on SF2 with base address 0x8
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0x3},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 10},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  /* use the WAN LED from runner */
  {bp_usSpeedLed100,           .u.us = BP_GPIO_26_AL}, 
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_27_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_14_AL},
  {bp_ulPhyId1,                .u.ul = GMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPortFlags,             .u.ul = PORT_FLAG_MGMT }, // Managment port is on switch
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Accessing SF2 as MMapped external switch
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = (BCM963138_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_0_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_1_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_28_AL},
  {bp_ulPhyId1,                .u.ul = (BCM963138_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_3_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_11_AL},
  {bp_ulPhyId2,                .u.ul = (BCM963138_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_5_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_12_AL},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ulCrossbar,              .u.ul = 13},
  {bp_ulCrossbarPhyId,         .u.ul = (BCM963138_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_6_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_7_AL},  
  {bp_usLinkLed,               .u.us = BP_GPIO_13_AL},
  {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16},
  {bp_last}
};

static bp_elem_t g_bcm963132ref_test[] = {
  {bp_cpBoardId,               .u.cp = "132REF_TEST"},
  {bp_ulMaxNumCpu,             .u.ul = 1},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963138ref_p802},
  {bp_last}
};

static bp_elem_t g_bcm963132ref_138_p502[] = {
  {bp_cpBoardId,               .u.cp = "132_138REF_P502"},
  {bp_ulMaxNumCpu,             .u.ul = 1},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963138ref_p502},
  {bp_last}
};

static bp_elem_t g_bcm963132ref_138gfast2[] = {
  {bp_cpBoardId,               .u.cp = "132_138GFAST2"},
  {bp_ulMaxNumCpu,             .u.ul = 1},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm963138ref_gfast_2},
  {bp_last}
};


bp_elem_t * g_BoardParms[] = {g_bcm963138sv, g_bcm963138dvt, g_bcm963138ref_gfast_p40x, g_bcm963138ref_gfast_2,  g_bcm963138ref_gfast_p40x_mem, g_bcm963138ref_gfast_4,
    g_bcm963138bg_gfast, g_bcm963138ref_p502, g_bcm963138ref_p802, g_bcm963138bmu_p202, g_bcm963138lte_p302, g_bcm963138lte2, g_bcm963138ref_p502_plc, g_bcm963138ref_p502_moca, 
    g_bcm963138ref_p502_bhr, g_bcm963138dvt_p300, g_bcm963138ref_rnc, g_bcm963138ref_rncp400, g_bcm963138ref_p502_moca_p200, g_bcm963138ref_p502_dual_moca_p200, g_bcm963138ref_3wlan, g_bcm963138ref_bgw, g_bcm963138rref_gfast, g_bcm963138rref_gfast_rgmii, g_bcm963138rref_rnc, g_bcm963138rref_rnc_rgmii, g_bcm963138dvt_serdes, g_bcm963138dvt_p300_serdes, g_bcm963138rref_7lan, g_bcm963132ref_test, g_bcm963132ref_138_p502, g_bcm963132ref_138gfast2, 0};


