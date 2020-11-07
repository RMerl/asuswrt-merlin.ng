#include "bp_defs.h"
#include "boardparms.h"
#include "bcmSpiRes.h"
extern BpCmdElem moca6802InitSeq[];


#define BCM947622_PHY_BASE             0x8

static bp_elem_t g_bcm947622sv[] = {
    {bp_cpBoardId,               .u.cp = "947622SV"},
    {bp_ulCompatChipId,          .u.ul = 0x47622},
    {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND},
    {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_EMMC},
    {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_HS_SPI},
    {bp_usButtonIdx,             .u.us = 0},
    {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
    {bp_usGpio_Intr,             .u.us = BP_GPIO_39_AL},
    {bp_usButtonAction,          .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
    {bp_ulButtonActionParm,      .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
    {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
    {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},
    {bp_usButtonIdx,             .u.us = 1},
    {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_1|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
    {bp_usGpio_Intr,             .u.us = BP_GPIO_38_AL},
    {bp_usButtonAction,          .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },

    {bp_usGphyBaseAddress,       .u.us = BCM947622_PHY_BASE}, 
    {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Cross bar switch
    {bp_ulPortMap,               .u.ul = 0x3},
    {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
    {bp_ulCrossbar,              .u.ul = BP_CROSSBAR_PORT_BASE},
    {bp_ulCrossbarPhyId,         .u.ul =  (BCM947622_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
    {bp_ulPhyId1,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
    {bp_ulCrossbar,              .u.ul = BP_CROSSBAR_PORT_BASE+1},
    {bp_ulCrossbarPhyId,         .u.ul = 0x18 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},
    {bp_ulCrossbar,              .u.ul = BP_CROSSBAR_PORT_BASE+2},
    {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},

    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_2_AL},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_8_AL},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_9_AL},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_10_AL},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_11_AL},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_12_AL},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_13_AL},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_14_AL},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_15_AL},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_16_AL},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_17_AL},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_18_AL},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_19_AL},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_20_AL},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_21_AL},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_22_AL},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_23_AL},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_24_AL},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_25_AL},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_26_AL},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_27_AL},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_28_AL},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_29_AL},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_30_AL},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_31_AL},

    {bp_usIntfId,                .u.us = 0},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
    {bp_usPortNum,               .u.us = 0},
    {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_0_AL},
    {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
    {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_1_AL},
    {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
    {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_3_AL},
    {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
    {bp_usIntfEnd},

    {bp_usIntfId,                .u.us = 1},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_UART},
    {bp_usPortNum,               .u.us = 1},
    {bp_usUartSdin,              .u.us = BP_GPIO_5_AH},
    {bp_usUartSdout,             .u.us = BP_GPIO_6_AH},
    {bp_usUartCts,               .u.us = BP_GPIO_3_AH}, 
    {bp_usUartRts,               .u.us = BP_GPIO_4_AH},
    {bp_usIntfEnd},

    {bp_usIntfId,                .u.us = 2},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
    {bp_usPortNum,               .u.us = 1},
    {bp_usNetLed0,               .u.us = BP_SERIAL_GPIO_4_AL},
    {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_2500},
    {bp_usNetLed1,               .u.us = BP_SERIAL_GPIO_5_AL},
    {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G|BP_NET_LED_SPEED_100},
    {bp_usNetLed2,               .u.us = BP_SERIAL_GPIO_6_AL},
    {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_2500},
    {bp_usNetLed3,               .u.us = BP_SERIAL_GPIO_7_AL},
    {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
    {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
    {bp_usIntfMgmtBusNum,        .u.us = 0},
    {bp_usExtIntrOpticalModulePresence, .u.us = BP_EXT_INTR_2 | BP_EXT_INTR_TYPE_IRQ_BOTH_EDGE | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
    {bp_usGpio_Intr,             .u.us = BP_GPIO_9_AL},
    {bp_usGpioSfpModDetect,      .u.us = BP_GPIO_9_AL},
    {bp_usSfpSigDetect,          .u.us = BP_GPIO_26_AL },
    {bp_usIntfEnd},

    {bp_usIntfId,                .u.us = 3},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
    {bp_usPortNum,               .u.us = 0},
    {bp_usGpioI2cSda,            .u.us = BP_GPIO_16_AH},
    {bp_usGpioI2cScl,            .u.us = BP_GPIO_17_AH},
    {bp_usIntfEnd},

     /* Serial LED must be AH in the new LED controllre. But the
        board design use AL so won't expect them to work... */
    {bp_usSerialLedData,         .u.us = BP_GPIO_0_AH},
    {bp_usSerialLedClk,          .u.us = BP_GPIO_1_AH},
    {bp_usSerialLedMask,         .u.us = BP_GPIO_2_AH},
    {bp_usSerialLedShiftOrder,   .u.us = BP_SERIAL_LED_SHIFT_MSB_FIRST},

    {bp_usI2sSclk,               .u.us = BP_GPIO_11_AH},
    {bp_usI2sLrck,               .u.us = BP_GPIO_12_AH},
    {bp_usI2sRxSdata,            .u.us = BP_GPIO_13_AH},
    {bp_usI2sMclk,               .u.us = BP_GPIO_14_AH},
    {bp_usI2sTxSdata,            .u.us = BP_GPIO_15_AH},
    {bp_usPcmSdin,               .u.us = BP_GPIO_22_AH},
    {bp_usPcmSdout,              .u.us = BP_GPIO_23_AH},
    {bp_usPcmClk,                .u.us = BP_GPIO_24_AH},
    {bp_usPcmFs,                 .u.us = BP_GPIO_25_AH},
    {bp_usGpioBtWake,            .u.us = BP_GPIO_27_AH},
    {bp_usGpioBtReset,           .u.us = BP_GPIO_7_AH},
    {bp_usSpiSlaveSelectNum,     .u.us = 0},
    {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_75_AL},
    {bp_usSpiSlaveSelectNum,     .u.us = 1},
    {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_76_AL},
    {bp_usSpiSlaveSelectNum,     .u.us = 2},
    {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_10_AL},  
    {bp_usUsbPwrOn0,             .u.us = BP_GPIO_80_AL},
    {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_79_AL},
    {bp_usUsbPwrOn1,             .u.us = BP_GPIO_82_AL},
    {bp_usUsbPwrFlt1,            .u.us = BP_GPIO_81_AL},

    {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
    {bp_ucDspAddress,            .u.uc = 0},
    {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_1067_14_14_14 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_SSC_CONFIG_1},
    {bp_last}
};

static bp_elem_t g_bcm947622sv_otp[] = {
    {bp_cpBoardId,               .u.cp = "947622SV_OTP"},

    {bp_usGphyBaseAddress,       .u.us = BCM947622_PHY_BASE}, 
    {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Cross bar switch
    {bp_ulPortMap,               .u.ul = 0x3},
    {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
    {bp_ulCrossbar,              .u.ul = BP_CROSSBAR_PORT_BASE},
    {bp_ulCrossbarPhyId,         .u.ul =  (BCM947622_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
    {bp_ulPhyId1,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
    {bp_ulCrossbar,              .u.ul = BP_CROSSBAR_PORT_BASE+1},
    {bp_ulCrossbarPhyId,         .u.ul = 0x18 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},

    {bp_elemTemplate,            .u.bp_elemp = g_bcm947622sv},
    {bp_last}
};

static bp_elem_t g_bcm947622sv_serdes[] = {
    {bp_cpBoardId,               .u.cp = "947622SV_SERDES"},	// deprecated use 947622sv instead
    {bp_elemTemplate,            .u.bp_elemp = g_bcm947622sv},
    {bp_last}
};

static bp_elem_t g_bcm947622rfdvt[] = {
    {bp_cpBoardId,               .u.cp = "947622RFDVT"},
    {bp_ulCompatChipId,          .u.ul = 0x47622},
    {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND},
    {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_HS_SPI},
    {bp_usButtonIdx,             .u.us = 0},
    {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
    {bp_usGpio_Intr,             .u.us = BP_GPIO_39_AL},
    {bp_usButtonAction,          .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
    {bp_ulButtonActionParm,      .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
    {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
    {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},
    {bp_usButtonIdx,             .u.us = 1},
    {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_1|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
    {bp_usGpio_Intr,             .u.us = BP_GPIO_38_AL},
    {bp_usButtonAction,          .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },

    {bp_usGphyBaseAddress,       .u.us = BCM947622_PHY_BASE}, 
    {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Cross bar switch
    {bp_ulPortMap,               .u.ul = 0x3},
    {bp_ulPhyId0,                .u.ul = (BCM947622_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
    {bp_ulPhyId1,                .u.ul = 0x18 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},

    {bp_usIntfId,                .u.us = 0},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
    {bp_usPortNum,               .u.us = 0},
    {bp_usNetLed0,               .u.us = BP_GPIO_0_AL},
    {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10},
    {bp_usNetLed1,               .u.us = BP_GPIO_1_AL},
    {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
    {bp_usNetLed2,               .u.us = BP_GPIO_2_AL},
    {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
    {bp_usNetLed3,               .u.us = BP_GPIO_3_AL},
    {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
    {bp_usIntfEnd},

    {bp_usIntfId,                .u.us = 1},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_xMII},
    {bp_usPortNum,               .u.us = 1},
    {bp_usNetLed0,               .u.us = BP_GPIO_4_AL},
    {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10},
    {bp_usNetLed1,               .u.us = BP_GPIO_5_AL},
    {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
    {bp_usNetLed2,               .u.us = BP_GPIO_6_AL},
    {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
    {bp_usNetLed3,               .u.us = BP_GPIO_7_AL},
    {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
    {bp_usIntfEnd},

    {bp_usSpiSlaveSelectNum,     .u.us = 0},
    {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_75_AL},

    {bp_usUsbPwrOn0,             .u.us = BP_GPIO_80_AL},
    {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_79_AL},
    {bp_usUsbPwrFlt1,            .u.us = BP_GPIO_81_AL},

    {bp_ulMemoryConfig,          .u.ul = BP_DDR_TYPE_DDR4 | BP_DDR_SPEED_1067_15_15_15 | BP_DDR_TOTAL_SIZE_2048MB| BP_DDR_DEVICE_WIDTH_8 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_SSC_CONFIG_1},

    {bp_last}
};

static bp_elem_t g_bcm947622rfdvt2[] = {
    {bp_cpBoardId,               .u.cp = "947622RFDVT2"},
    {bp_usGphyBaseAddress,       .u.us = BCM947622_PHY_BASE}, 
    {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Cross bar switch
    {bp_ulPortMap,               .u.ul = 0x3},
    {bp_ulPhyId0,                .u.ul = (BCM947622_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
    {bp_ulPhyId1,                .u.ul = 6 | PHY_INTEGRATED_VALID | MAC_IF_SERDES},

    {bp_usIntfId,                .u.us = 1},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
    {bp_usPortNum,               .u.us = 1},
    {bp_usNetLed0,               .u.us = BP_GPIO_4_AL},
    {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_10},
    {bp_usNetLed1,               .u.us = BP_GPIO_5_AL},
    {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100},
    {bp_usNetLed2,               .u.us = BP_GPIO_6_AL},
    {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
    {bp_usNetLed3,               .u.us = BP_GPIO_7_AL},
    {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
    {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
    {bp_usIntfMgmtBusNum,        .u.us = 0},
    {bp_usExtIntrOpticalModulePresence, .u.us = BP_EXT_INTR_2 | BP_EXT_INTR_TYPE_IRQ_BOTH_EDGE | BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE},
    {bp_usGpio_Intr,             .u.us = BP_GPIO_9_AL},
    {bp_usGpioSfpModDetect,      .u.us = BP_GPIO_9_AL},
    {bp_usSfpSigDetect,          .u.us = BP_GPIO_26_AL},
    {bp_usIntfEnd},
    {bp_usIntfId,                .u.us = 2},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
    {bp_usPortNum,               .u.us = 0},
    {bp_usGpioI2cSda,            .u.us = BP_GPIO_16_AH},
    {bp_usGpioI2cScl,            .u.us = BP_GPIO_17_AH},
    {bp_usIntfEnd},

    {bp_usUsbPwrOn0,             .u.us = BP_GPIO_80_AL},
    {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_79_AL},
    {bp_usUsbPwrOn1,             .u.us = BP_GPIO_82_AL},
    {bp_usUsbPwrFlt1,            .u.us = BP_GPIO_81_AL},

    {bp_elemTemplate,            .u.bp_elemp = g_bcm947622rfdvt},
    {bp_last}
};

static bp_elem_t g_bcm947622eap[] = {
    {bp_cpBoardId,               .u.cp = "947622EAP"},
    {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND},

    {bp_usButtonIdx,             .u.us = 0},
    {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
    {bp_usGpio_Intr,             .u.us = BP_GPIO_4_AL},
    {bp_usButtonAction,          .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },

    {bp_usGphyBaseAddress,       .u.us = BCM947622_PHY_BASE}, 
    {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Cross bar switch
    {bp_ulPortMap,               .u.ul = 0x3},
    {bp_ulPhyId0,                .u.ul = (BCM947622_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
    {bp_ulPhyId1,                .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES | EXTSW_CONNECTED},

    {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
    {bp_ucPhyAddress,            .u.uc = 0x0},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MDIO},
    {bp_ulPortMap,               .u.ul = 0x0f},
    {bp_ulPhyId0,                .u.ul = BP_PHY_ID_0 | CONNECTED_TO_EXTERN_SW},
    {bp_ulPhyId1,                .u.ul = BP_PHY_ID_1 | CONNECTED_TO_EXTERN_SW},
    {bp_ulPhyId2,                .u.ul = BP_PHY_ID_2 | CONNECTED_TO_EXTERN_SW},
    {bp_ulPhyId3,                .u.ul = BP_PHY_ID_3 | CONNECTED_TO_EXTERN_SW},

    {bp_usIntfId,                .u.us = 0},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
    {bp_usPortNum,               .u.us = 0},
    {bp_usNetLed0,               .u.us = BP_GPIO_0_AL},
    {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
    {bp_usNetLed3,               .u.us = BP_GPIO_3_AL},
    {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
    {bp_usIntfEnd},

    {bp_ulMemoryConfig,          .u.ul = BP_DDR_TYPE_DDR4 | BP_DDR_SPEED_1067_15_15_15 | BP_DDR_TOTAL_SIZE_2048MB| BP_DDR_DEVICE_WIDTH_8 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_SSC_CONFIG_1},

    {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_HS_SPI},
    {bp_usSpiSlaveSelectNum,     .u.us = 0},
    {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_75_AL},
    {bp_usSpiSlaveSelectNum,     .u.us = 1},
    {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_76_AL},

    /* Use PINMUX 0 for BT coex */
    {bp_usGpioWlanReserved1,     .u.us = BP_GPIO_30_AH}, 
    {bp_usGpioWlanReserved1,     .u.us = BP_GPIO_31_AH},
    {bp_usGpioWlanReserved1,     .u.us = BP_GPIO_32_AH},

    /* reset-gpio for th external switch. */
    {bp_usGpioInitState,         .u.us = BP_GPIO_10_AH},
    {bp_last}
};

static bp_elem_t g_bcm947622leap[] = {
    {bp_cpBoardId,               .u.cp = "947622LEAP"},
    {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND},

    {bp_usButtonIdx,             .u.us = 0},
    {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
    {bp_usGpio_Intr,             .u.us = BP_GPIO_4_AL},
    {bp_usButtonAction,          .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },

    {bp_usGphyBaseAddress,       .u.us = BCM947622_PHY_BASE},
    {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Cross bar switch
	{bp_ucPhyAddress,            .u.uc = 0x1},
    {bp_ulPortMap,               .u.ul = 0x1},
    {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
    {bp_ulCrossbar,              .u.ul = BP_CROSSBAR_PORT_BASE},
    {bp_ulCrossbarPhyId,         .u.ul = (BCM947622_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},

    {bp_usIntfId,                .u.us = 0},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
    {bp_usPortNum,               .u.us = 0},
    {bp_usNetLed0,               .u.us = BP_GPIO_0_AL},
    {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
    {bp_usNetLed3,               .u.us = BP_GPIO_3_AL},
    {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
    {bp_usIntfEnd},

    {bp_ulMemoryConfig,          .u.ul = BP_DDR_TYPE_DDR4 | BP_DDR_SPEED_1067_15_15_15 | BP_DDR_TOTAL_SIZE_2048MB| BP_DDR_DEVICE_WIDTH_8 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_SSC_CONFIG_1},

    {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_HS_SPI},
    {bp_usSpiSlaveSelectNum,     .u.us = 0},
    {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_75_AL},
    {bp_usSpiSlaveSelectNum,     .u.us = 1},
    {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_76_AL},

    {bp_last}
};

static bp_elem_t g_bcm947622eap2[] = {
    {bp_cpBoardId,               .u.cp = "947622EAP2"},
    {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND},

    {bp_usButtonIdx,             .u.us = 0},
    {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
    {bp_usGpio_Intr,             .u.us = BP_GPIO_4_AL},
    {bp_usButtonAction,          .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },

    {bp_usGphyBaseAddress,       .u.us = BCM947622_PHY_BASE},
    {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Cross bar switch
	{bp_ucPhyAddress,            .u.uc = 0x1},
    {bp_ulPortMap,               .u.ul = 0x3},
    {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
    {bp_ulCrossbar,              .u.ul = BP_CROSSBAR_PORT_BASE},
    {bp_ulCrossbarPhyId,         .u.ul = (BCM947622_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},

    {bp_ulPhyId1,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
    {bp_ulCrossbar,              .u.ul = BP_CROSSBAR_PORT_BASE+2},
    {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
	{bp_ulCrossbarPhyId,         .u.ul = 0x1 | PHY_EXTERNAL | PHY_TYPE_CL45GPHY},
	{bp_ulPortFlags,             .u.ul = PORT_FLAG_SWAP_PAIR},
    {bp_usGpioPhyReset,          .u.us = BP_GPIO_33_AL},

    {bp_usIntfId,                .u.us = 0},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
    {bp_usPortNum,               .u.us = 0},
    {bp_usNetLed0,               .u.us = BP_GPIO_0_AL},
    {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
    {bp_usNetLed3,               .u.us = BP_GPIO_3_AL},
    {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
    {bp_usIntfEnd},

    {bp_ulMemoryConfig,          .u.ul = BP_DDR_TYPE_DDR4 | BP_DDR_SPEED_1067_15_15_15 | BP_DDR_TOTAL_SIZE_2048MB| BP_DDR_DEVICE_WIDTH_8 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_SSC_CONFIG_1},

    {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_HS_SPI},
    {bp_usSpiSlaveSelectNum,     .u.us = 0},
    {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_75_AL},
    {bp_usSpiSlaveSelectNum,     .u.us = 1},
    {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_76_AL},

    {bp_last}
};
static bp_elem_t g_bcm947622eaps[] = {
    {bp_cpBoardId,               .u.cp = "947622EAPS"},
    {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND},

    {bp_usButtonIdx,             .u.us = 0},
    {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
    {bp_usGpio_Intr,             .u.us = BP_GPIO_4_AL},
    {bp_usButtonAction,          .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },

    {bp_usGphyBaseAddress,       .u.us = BCM947622_PHY_BASE}, 
    {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Cross bar switch
	{bp_ucPhyAddress,            .u.uc = 0x1e},
    {bp_ulPortMap,               .u.ul = 0x3},
    {bp_ulPhyId0,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
    {bp_ulCrossbar,              .u.ul = BP_CROSSBAR_PORT_BASE},
    {bp_ulCrossbarPhyId,         .u.ul = (BCM947622_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},

    {bp_ulPhyId1,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
    {bp_ulCrossbar,              .u.ul = BP_CROSSBAR_PORT_BASE+2},
    {bp_ulCrossbarPhyId,         .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
	{bp_ulCrossbarPhyId,         .u.ul = 0x1e | PHY_EXTERNAL | PHY_TYPE_CL45GPHY},
	{bp_ulPortFlags,             .u.ul = PORT_FLAG_SWAP_PAIR},
    {bp_usGpioPhyReset,          .u.us = BP_GPIO_33_AL},

    {bp_usIntfId,                .u.us = 0},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
    {bp_usPortNum,               .u.us = 0},
    {bp_usNetLed0,               .u.us = BP_GPIO_0_AL},
    {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_1G},
    {bp_usNetLed3,               .u.us = BP_GPIO_3_AL},
    {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
    {bp_usIntfEnd},

    {bp_ulMemoryConfig,          .u.ul = BP_DDR_TYPE_DDR4 | BP_DDR_SPEED_1067_15_15_15 | BP_DDR_TOTAL_SIZE_2048MB| BP_DDR_DEVICE_WIDTH_8 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_SSC_CONFIG_1},

    {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_HS_SPI},
    {bp_usSpiSlaveSelectNum,     .u.us = 0},
    {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_75_AL},
    {bp_usSpiSlaveSelectNum,     .u.us = 1},
    {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_76_AL},

    {bp_last}
};

static bp_elem_t g_bcm947622ref1[] = {
    {bp_cpBoardId,               .u.cp = "947622REF1"},
    {bp_ulCompatChipId,          .u.ul = 0x47622},
    {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND},
    {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_HS_SPI},
    {bp_usButtonIdx,             .u.us = 0},
    {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
    {bp_usGpio_Intr,             .u.us = BP_GPIO_4_AL},
    {bp_usButtonAction,          .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },

    {bp_usGpioLedWanLink,        .u.us = BP_GPIO_5_AL},   /* nobody use this led but defined in board*/
    {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_1_AL},   /* WPS_LED */
    {bp_usGpioLedWL1Act,         .u.us = BP_GPIO_7_AL | BP_LED_USE_GPIO},   /* WLAN_5G_ACT */
    {bp_usGpioLedWL0Act,         .u.us = BP_GPIO_36_AL | BP_LED_USE_GPIO},  /* WLAN_2G_ACT, must use BP_LED_USE_GPIO */

    {bp_usGphyBaseAddress,       .u.us = BCM947622_PHY_BASE}, 
    {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Cross bar switch
    {bp_ulPortMap,               .u.ul = 0x3},
    {bp_ulPhyId0,                .u.ul = (BCM947622_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
    {bp_ulPhyId1,                .u.ul = RGMII_DIRECT_3P3V | EXTSW_CONNECTED},
    {bp_ulPortFlags,             .u.ul = PORT_FLAG_TX_INTERNAL_DELAY | PORT_FLAG_RX_INTERNAL_DELAY},
    {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
    {bp_ucPhyAddress,            .u.uc = 0x0},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MDIO},
    {bp_ulPortMap,               .u.ul = 0x0f},
    {bp_ulPhyId0,                .u.ul = BP_PHY_ID_0 | CONNECTED_TO_EXTERN_SW},
    {bp_ulPhyId1,                .u.ul = BP_PHY_ID_1 | CONNECTED_TO_EXTERN_SW},
    {bp_ulPhyId2,                .u.ul = BP_PHY_ID_2 | CONNECTED_TO_EXTERN_SW},
    {bp_ulPhyId3,                .u.ul = BP_PHY_ID_3 | CONNECTED_TO_EXTERN_SW},

    {bp_usIntfId,                .u.us = 0},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
    {bp_usPortNum,               .u.us = 0},
    {bp_usNetLed0,               .u.us = BP_GPIO_0_AH},
    {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_1G},
    {bp_usNetLed3,               .u.us = BP_GPIO_3_AL},
    {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
    {bp_usIntfEnd},

    {bp_usIntfId,                .u.us = 1},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
    {bp_usPortNum,               .u.us = 0},
    {bp_usGpioI2cSda,            .u.us = BP_GPIO_16_AH},
    {bp_usGpioI2cScl,            .u.us = BP_GPIO_17_AH},
    {bp_usIntfEnd},

    /* may also bue used as LED channels for RGB LED test */
    {bp_usI2sSclk,               .u.us = BP_GPIO_11_AH},
    {bp_usI2sLrck,               .u.us = BP_GPIO_12_AH},
    {bp_usI2sRxSdata,            .u.us = BP_GPIO_13_AH},
    {bp_usI2sMclk,               .u.us = BP_GPIO_14_AH},
    {bp_usI2sTxSdata,            .u.us = BP_GPIO_15_AH},
    {bp_usPcmSdin,               .u.us = BP_GPIO_22_AH},
    {bp_usPcmSdout,              .u.us = BP_GPIO_23_AH},
    {bp_usPcmClk,                .u.us = BP_GPIO_24_AH},
    {bp_usPcmFs,                 .u.us = BP_GPIO_25_AH},

    {bp_usMiiMdc,                .u.us = BP_GPIO_68_AH},
    {bp_usMiiMdio,               .u.us = BP_GPIO_69_AH},
    {bp_usSpiSlaveSelectNum,     .u.us = 0},
    {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_75_AL},
    {bp_usSpiSlaveSelectNum,     .u.us = 1},
    {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_76_AL},
    {bp_usSpiSlaveSelectNum,     .u.us = 2},
    {bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_10_AL},  

    {bp_usUsbPwrOn0,             .u.us = BP_GPIO_80_AH},
    {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_79_AL},
    {bp_usUsbPwrOn1,             .u.us = BP_GPIO_82_AH},
    {bp_usUsbPwrFlt1,            .u.us = BP_GPIO_81_AL},

    {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
    {bp_ucDspAddress,            .u.uc = 0},
    {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_1067_14_14_14 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_SSC_CONFIG_1},
    {bp_last}
};

static bp_elem_t g_bcm947622ref1_sg[] = {
    {bp_cpBoardId,               .u.cp = "947622REF1_SG"},
    {bp_usGphyBaseAddress,       .u.us = BCM947622_PHY_BASE}, 
    {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Cross bar switch
    {bp_ulPortMap,               .u.ul = 0x3},
    {bp_ulPhyId0,                .u.ul = (BCM947622_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
    {bp_ulPhyId1,                .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES | EXTSW_CONNECTED},
    {bp_elemTemplate,            .u.bp_elemp = g_bcm947622ref1},
    {bp_last}
};

static bp_elem_t g_bcm947622ref1_p250[] = {
    {bp_cpBoardId,               .u.cp = "947622REF1P250"},
    {bp_usGpioInitState,         .u.us = BP_GPIO_10_AH},
    {bp_usIntfId,                .u.us = 0},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
    {bp_usPortNum,               .u.us = 0},
    {bp_usNetLed0,               .u.us = BP_GPIO_0_AL},
    {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_1G},
    {bp_usNetLed3,               .u.us = BP_GPIO_3_AL},
    {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
    {bp_usIntfEnd},
    {bp_elemTemplate,            .u.bp_elemp = g_bcm947622ref1},
    {bp_last}
};

static bp_elem_t g_bcm947622ref1_sg_p250[] = {
    {bp_cpBoardId,               .u.cp = "47622REF1SGP250"},
    {bp_usGpioInitState,         .u.us = BP_GPIO_10_AH},
    {bp_usIntfId,                .u.us = 0},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
    {bp_usPortNum,               .u.us = 0},
    {bp_usNetLed0,               .u.us = BP_GPIO_0_AL},
    {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_1G},
    {bp_usNetLed3,               .u.us = BP_GPIO_3_AL},
    {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
    {bp_usIntfEnd},
    {bp_elemTemplate,            .u.bp_elemp = g_bcm947622ref1_sg},
    {bp_last}
};

static bp_elem_t g_bcm96755ref1[] = {
    {bp_cpBoardId,               .u.cp = "96755REF1"},
    {bp_ulCompatChipId,          .u.ul = 0x47622},  /* remove this line when chip id is otp'ed to 0x6755 */
    {bp_ulCompatChipId,          .u.ul = 0x6755},
    /* use slower ddr clock in 6755 chip for now. May bump up to 1067MHz if test pass */
    {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_SSC_CONFIG_1},
    {bp_elemTemplate,            .u.bp_elemp = g_bcm947622ref1},
    {bp_last}
};

static bp_elem_t g_bcm96755ref1_sg[] = {
    {bp_cpBoardId,               .u.cp = "96755REF1_SG"},
    {bp_ulCompatChipId,          .u.ul = 0x47622},  /* remove this line when chip id is otp'ed to 0x6755 */
    {bp_ulCompatChipId,          .u.ul = 0x6755},
    /* use slower ddr clock in 6755 chip for now. May bump up to 1067MHz if test pass */
    {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_SSC_CONFIG_1},
    {bp_elemTemplate,            .u.bp_elemp = g_bcm947622ref1_sg},
    {bp_last}
};

static bp_elem_t g_bcm96755ref1_p200[] = {
    {bp_cpBoardId,               .u.cp = "96755REF1P200"},
    {bp_usGpioInitState,         .u.us = BP_GPIO_10_AH},
    {bp_usIntfId,                .u.us = 0},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
    {bp_usPortNum,               .u.us = 0},
    {bp_usNetLed0,               .u.us = BP_GPIO_0_AL},
    {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_1G},
    {bp_usNetLed3,               .u.us = BP_GPIO_3_AL},
    {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
    {bp_usIntfEnd},
    {bp_elemTemplate,            .u.bp_elemp = g_bcm96755ref1},
    {bp_last}
};

static bp_elem_t g_bcm96755ref1_sg_p200[] = {
    {bp_cpBoardId,               .u.cp = "96755REF1SGP200"},
    {bp_usGpioInitState,         .u.us = BP_GPIO_10_AH},
    {bp_usGpioWlanReserved,      .u.us = BP_GPIO_30_AH},
    {bp_usGpioWlanReserved,      .u.us = BP_GPIO_31_AH},
    {bp_usGpioWlanReserved,      .u.us = BP_GPIO_32_AH},
    {bp_usIntfId,                .u.us = 0},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
    {bp_usPortNum,               .u.us = 0},
    {bp_usNetLed0,               .u.us = BP_GPIO_0_AL},
    {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_1G},
    {bp_usNetLed3,               .u.us = BP_GPIO_3_AL},
    {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
    {bp_usIntfEnd},
    {bp_elemTemplate,            .u.bp_elemp = g_bcm96755ref1_sg},
    {bp_last}
};

static bp_elem_t g_bcm96755tbrhx[] = {
    {bp_cpBoardId,               .u.cp = "96755TBRHX"},
    {bp_ulCompatChipId,          .u.ul = 0x47622},  /* remove this line when chip id is otp'ed to 0x6755 */
    {bp_ulCompatChipId,          .u.ul = 0x6755},

    {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
    {bp_usGpio_Intr,             .u.us = BP_GPIO_10_AL},
    {bp_usButtonAction,          .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
    {bp_ulButtonActionParm,      .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
    {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
    {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},
    {bp_usButtonIdx,             .u.us = 1},
    {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_1|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
    {bp_usGpio_Intr,             .u.us = BP_GPIO_8_AL},
    {bp_usButtonAction,          .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },

    {bp_usGphyBaseAddress,       .u.us = BCM947622_PHY_BASE}, 
    {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Cross bar switch
    {bp_ulPortMap,               .u.ul = 0x3},
    {bp_ulPhyId0,                .u.ul = (BCM947622_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
    {bp_ulPhyId1,                .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
    {bp_ulPhyId1,                .u.ul = 0x1f | PHY_EXTERNAL | PHY_TYPE_CL45GPHY},
    {bp_ulPortFlags,             .u.ul = PORT_FLAG_SWAP_PAIR},


    {bp_usIntfId,                .u.us = 0},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_GPHY},
    {bp_usPortNum,               .u.us = 0},
    {bp_usNetLed0,               .u.us = BP_GPIO_25_AL},
    {bp_ulNetLedLink,            .u.ul = BP_NET_LED_SPEED_100|BP_NET_LED_SPEED_10|BP_NET_LED_SPEED_1G},
    {bp_usNetLed3,               .u.us = BP_GPIO_28_AL},
    {bp_ulNetLedActivity,        .u.ul = BP_NET_LED_ACTIVITY_ALL},
    {bp_usIntfEnd},

    {bp_usIntfId,                .u.us = 1},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_UART},
    {bp_usPortNum,               .u.us = 1},
    {bp_usUartSdin,              .u.us = BP_GPIO_5_AH},
    {bp_usUartSdout,             .u.us = BP_GPIO_6_AH},
    {bp_usUartCts,               .u.us = BP_GPIO_3_AH}, 
    {bp_usUartRts,               .u.us = BP_GPIO_4_AH},
    {bp_usIntfEnd},

    {bp_usI2sSclk,               .u.us = BP_GPIO_11_AH},
    {bp_usI2sLrck,               .u.us = BP_GPIO_12_AH},
    {bp_usI2sRxSdata,            .u.us = BP_GPIO_13_AH},
    {bp_usI2sMclk,               .u.us = BP_GPIO_14_AH},
    {bp_usI2sTxSdata,            .u.us = BP_GPIO_15_AH},

    {bp_usIntfId,                .u.us = 2},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
    {bp_usPortNum,               .u.us = 0},
    {bp_usGpioI2cSda,            .u.us = BP_GPIO_16_AH},
    {bp_usGpioI2cScl,            .u.us = BP_GPIO_17_AH},
    {bp_usIntfEnd},

    {bp_usUsbPwrOn0,             .u.us = BP_GPIO_80_AH},
    {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_79_AL},
    {bp_usUsbPwrOn1,             .u.us = BP_GPIO_82_AH},
    {bp_usUsbPwrFlt1,            .u.us = BP_GPIO_81_AL},

    {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_7_AL},
 

    {bp_usGpioBtWake,            .u.us = BP_GPIO_33_AH},
    {bp_usGpioBtReset,           .u.us = BP_GPIO_29_AH},

    {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_1067_14_14_14 | BP_DDR_TOTAL_SIZE_512MB | BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_SSC_CONFIG_1},
    {bp_last}
};

static bp_elem_t g_bcm96755ref2[] = {
    {bp_cpBoardId,               .u.cp = "96755REF2"},
    {bp_ulCompatChipId,          .u.ul = 0x6755},
    {bp_ulCompatChipId,          .u.ul = 0x47622},
     /* Serial LED must be AH in the new LED controllre. But the
        board design use AL so won't expect them to work... */
    {bp_usSerialLedData,         .u.us = BP_GPIO_0_AH},
    {bp_usSerialLedClk,          .u.us = BP_GPIO_1_AH},
    {bp_usSerialLedMask,         .u.us = BP_GPIO_2_AH},

    {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND},
    {bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_HS_SPI},
    {bp_usButtonIdx,             .u.us = 0},
    {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_1|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
    {bp_usGpio_Intr,             .u.us = BP_GPIO_38_AL},
    {bp_usButtonAction,          .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_PRESS  },
    {bp_usButtonIdx,             .u.us = 1},
    {bp_usButtonExtIntr,         .u.us = BP_EXT_INTR_0|BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE|BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
    {bp_usGpio_Intr,             .u.us = BP_GPIO_39_AL},
    {bp_usButtonAction,          .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
    {bp_ulButtonActionParm,      .u.ptr = (void*)"Button Press -- Hold for 5s to do restore to default" },
    {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESET | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
    {bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S},

    {bp_usGphyBaseAddress,       .u.us = BCM947622_PHY_BASE}, 
    {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP}, // Cross bar switch
    {bp_ulPortMap,               .u.ul = 0x1},
    {bp_ulPhyId0,                .u.ul = (BCM947622_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},

    {bp_usIntfId,                .u.us = 0},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
    {bp_usPortNum,               .u.us = 0},
    {bp_usGpioI2cSda,            .u.us = BP_GPIO_16_AH},
    {bp_usGpioI2cScl,            .u.us = BP_GPIO_17_AH},
    {bp_usIntfEnd},

    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_0_AH},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_1_AH},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_2_AH},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_3_AH},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_4_AH},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_5_AH},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_6_AH},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_7_AH},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_8_AH},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_9_AH},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_10_AH},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_11_AH},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_12_AH},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_13_AH},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_14_AH},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_15_AH},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_16_AH},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_17_AH},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_18_AH},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_19_AH},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_20_AH},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_21_AH},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_22_AH},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_23_AH},
    {bp_usGpioLedReserved,       .u.us = BP_SERIAL_GPIO_24_AH},


    /* may also bue used as LED channels for RGB LED test */
    {bp_usI2sSclk,               .u.us = BP_GPIO_11_AH},
    {bp_usI2sLrck,               .u.us = BP_GPIO_12_AH},
    {bp_usI2sRxSdata,            .u.us = BP_GPIO_13_AH},
    {bp_usI2sMclk,               .u.us = BP_GPIO_14_AH},
    {bp_usI2sTxSdata,            .u.us = BP_GPIO_15_AH},

    {bp_usUsbPwrOn0,             .u.us = BP_GPIO_80_AL},
    {bp_usUsbPwrFlt0,            .u.us = BP_GPIO_79_AL},

    {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_SSC_CONFIG_1},
    {bp_last}
};

static bp_elem_t g_bcm96755ref1t_sg[] = {
    {bp_cpBoardId,               .u.cp = "96755REF1TSG"},

    {bp_ulMemoryConfig,          .u.ul = BP_DDR_SPEED_1067_14_14_14 | BP_DDR_TOTAL_SIZE_512MB | BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_16BIT | BP_DDR_SSC_CONFIG_1},
    {bp_elemTemplate,            .u.bp_elemp = g_bcm96755ref1_sg_p200},
    {bp_last}
};
	
bp_elem_t * g_BoardParms[] = {g_bcm947622sv, g_bcm947622sv_otp, g_bcm947622rfdvt, g_bcm947622rfdvt2, g_bcm947622ref1, g_bcm96755ref1, g_bcm947622eap, g_bcm947622eap2, g_bcm947622eaps, g_bcm947622sv_serdes, g_bcm947622ref1_sg, g_bcm96755ref1_sg, g_bcm96755tbrhx, \
    g_bcm947622ref1_p250, g_bcm947622ref1_sg_p250, g_bcm96755ref1_p200, g_bcm96755ref1_sg_p200, g_bcm96755ref2, g_bcm947622leap, g_bcm96755ref1t_sg, 0};


