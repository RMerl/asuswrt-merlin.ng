#include "bp_defs.h"
#include "boardparms.h"
#include "bcmSpiRes.h"

#define BCM963146_PHY_BASE             0x1

static bp_elem_t g_bcm963146sv_ddr4[] = {
	{bp_cpBoardId,               .u.cp = "963146SV_DDR4"},
	{bp_usSerialLedData,         .u.us = BP_GPIO_8_AH},
	{bp_usSerialLedClk,          .u.us = BP_GPIO_6_AH},
	{bp_usSerialLedMask,         .u.us = BP_GPIO_11_AH},
	{bp_ulInterfaceEnable,       .u.ul = BP_PINMUX_FNTYPE_NAND},

	{bp_usGphyBaseAddress,       .u.us = BCM963146_PHY_BASE},
	{bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
	{bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
	{bp_ucPhyAddress,            .u.uc = 0x1e},
	{bp_ulPortMap,               .u.ul = 0xdf}, // QGPHYs+SGPHY+AE+SGMII
	// QGPHYs
	{bp_ulPhyId0,                .u.ul = (BCM963146_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
	{bp_ulPhyId1,                .u.ul = (BCM963146_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
	{bp_ulPhyId2,                .u.ul = (BCM963146_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
	{bp_ulPhyId3,                .u.ul = (BCM963146_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
	// SGPHY
	{bp_ulPhyId4,                .u.ul = (BCM963146_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
	// Serdes
	{bp_ulPhyId6,                .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
	{bp_ulPhyId7,                .u.ul = 7 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},

	{bp_usIntfId,                .u.us = 0},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_UART},
	{bp_usPortNum,               .u.us = 5},
	{bp_usUartSdin,              .u.us = BP_GPIO_67_AH},
	{bp_usUartSdout,             .u.us = BP_GPIO_68_AH},
	{bp_usIntfEnd},
	{bp_usIntfId,                .u.us = 1},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
	{bp_usPortNum,               .u.us = 0},
	{bp_usGpioI2cSda,            .u.us = BP_GPIO_12_AH },
	{bp_usGpioI2cScl,            .u.us = BP_GPIO_3_AH },
	{bp_usIntfEnd},
	{bp_usIntfId,                .u.us = 2},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
	{bp_usPortNum,               .u.us = 1},
	{bp_usGpioI2cSda,            .u.us = BP_GPIO_25_AH },
	{bp_usGpioI2cScl,            .u.us = BP_GPIO_22_AH },
	{bp_usIntfEnd},

	{bp_usIntfId,                .u.us = 3},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
	{bp_usPortNum,               .u.us = 0},
	{bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6305 | BP_AFE_FE_REV_6305_REV_12_5_80 | BP_AFE_FE_RNC},
	{bp_usGpioAFELDPwr,          .u.us = BP_GPIO_87_AH}, // Line Driver 0 = "Int"
	{bp_usGpioAFELDData,         .u.us = BP_GPIO_17_AH},
	{bp_usGpioAFELDClk,          .u.us = BP_GPIO_34_AH},
	{bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_32_AH},
	{bp_usGpioAFELDRelay,        .u.us = BP_GPIO_31_AH},
	{bp_usIntfEnd},
	{bp_usIntfId,                .u.us = 4},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
	{bp_usPortNum,               .u.us = 1},
	{bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH1 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_146__REV_12_3_85 | BP_AFE_FE_RNC},
	{bp_usGpioAFELDPwr,          .u.us = BP_GPIO_33_AH}, // Line Driver 1 = "Ext"
	{bp_usGpioAFELDData,         .u.us = BP_GPIO_86_AH},
	{bp_usGpioAFELDClk,          .u.us = BP_GPIO_84_AH},
	{bp_usIntfEnd},	
	//secondary definitions for AfeIds
	{bp_usIntfId,                .u.us = 5},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
	{bp_usPortNum,               .u.us = 0},
	{bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_146__REV_12_3_60 | BP_AFE_FE_RNC},
	{bp_usIntfEnd},
	{bp_usIntfId,                .u.us = 6},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
	{bp_usPortNum,               .u.us = 1},
	{bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH1 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_146__REV_12_3_85 | BP_AFE_FE_RNC},
	{bp_usIntfEnd},
	/* Must keep the SGMII intf order as below. Do not shuffle or delete SGMII interface */
	{bp_usIntfId,                .u.us = 7},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
	{bp_usPortNum,               .u.us = 6},
	{bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
	{bp_usIntfMgmtBusNum,        .u.us = 1},
	{bp_usGpioSfpModDetect,      .u.us = BP_GPIO_30_AL},
	{bp_usSfpSigDetect,          .u.us = BP_GPIO_35_AL},
	{bp_usIntfEnd},
	{bp_usIntfId,                .u.us = 8},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
	{bp_usPortNum,               .u.us = 7},
	{bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
	{bp_usIntfMgmtBusNum,        .u.us = 0},
	{bp_usGpioSfpModDetect,      .u.us = BP_GPIO_16_AL},
	{bp_usSfpSigDetect,          .u.us = BP_GPIO_5_AL},
	{bp_usIntfEnd},
	
	{bp_usSpiSlaveSelectNum,     .u.us = 0},
	{bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_36_AL},
	{bp_usSpiSlaveSelectNum,     .u.us = 1},
	{bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_20_AL},

	{bp_usUsbPwrOn1,             .u.us = BP_GPIO_72_AL},
	{bp_usUsbPwrFlt1,            .u.us = BP_GPIO_71_AL},
	{bp_usUsbPwrOn0,             .u.us = BP_GPIO_70_AL},
	{bp_usUsbPwrFlt0,            .u.us = BP_GPIO_69_AL},

	{bp_usPcmSdin,               .u.us = BP_GPIO_80_AH},
	{bp_usPcmSdout,              .u.us = BP_GPIO_81_AH},
	{bp_usPcmClk,                .u.us = BP_GPIO_85_AH},
	{bp_usPcmFs,                 .u.us = BP_GPIO_82_AH},

	{bp_last}
};

static bp_elem_t g_bcm963146sv_ddr4_rgmii[] = {
	{bp_cpBoardId,               .u.cp = "63146SV_DDR4RGM"},
	{bp_usGphyBaseAddress,       .u.us = BCM963146_PHY_BASE},
	{bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
	{bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
	{bp_ucPhyAddress,            .u.uc = 0x1e},
	{bp_ulPortMap,               .u.ul = 0xff}, // QGPHYs+SGPHY+RGMII+AE+SGMII
	// QGPHYs
	{bp_ulPhyId0,                .u.ul = (BCM963146_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
	{bp_ulPhyId1,                .u.ul = (BCM963146_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
	{bp_ulPhyId2,                .u.ul = (BCM963146_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
	{bp_ulPhyId3,                .u.ul = (BCM963146_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
	// SGPHY
	{bp_ulPhyId4,                .u.ul = (BCM963146_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
	// RGMII
	{bp_ulPhyId5,                .u.ul =  0x19 | PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},
	// Serdes
	{bp_ulPhyId6,                .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},  //SGMII
	{bp_ulPhyId7,                .u.ul = 7 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},  //AE
  
	{bp_elemTemplate,            .u.bp_elemp = g_bcm963146sv_ddr4},  
	{bp_last}  
};

static bp_elem_t g_bcm963146sv[] = {
	{bp_cpBoardId,               .u.cp = "963146SV"},

	{bp_elemTemplate,            .u.bp_elemp = g_bcm963146sv_ddr4},  
	{bp_last}
};

static bp_elem_t g_bcm963146sv_rgmii[] = {
	{bp_cpBoardId,               .u.cp = "63146SV_RGM"},

	{bp_elemTemplate,            .u.bp_elemp = g_bcm963146sv_ddr4_rgmii},  
	{bp_last}
};


static bp_elem_t g_bcm963146ref1d[] = {
	{bp_cpBoardId,               .u.cp = "963146REF1D"},
	{bp_usSerialLedData,         .u.us = BP_GPIO_8_AH},
	{bp_usSerialLedClk,          .u.us = BP_GPIO_6_AH},
	{bp_usSerialLedMask,         .u.us = BP_GPIO_11_AH},

	{bp_usGphyBaseAddress,       .u.us = BCM963146_PHY_BASE},
	{bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
	{bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
	{bp_ucPhyAddress,            .u.uc = 0x1e},
	{bp_ulPortMap,               .u.ul = 0xdf}, //QGPHYs+SGPHY+AE+SGMII
	// QGPHYs
	{bp_ulPhyId0,                .u.ul = (BCM963146_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
	{bp_ulPhyId1,                .u.ul = (BCM963146_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
	{bp_ulPhyId2,                .u.ul = (BCM963146_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
	{bp_ulPhyId3,                .u.ul = (BCM963146_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
    // SGPHY
	{bp_ulPhyId4,                .u.ul = (BCM963146_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
	// Serdes
	{bp_ulPhyId6,                .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},    //SGMII
	{bp_ulPhyId7,                .u.ul = 7 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},    //AE

	{bp_usIntfId,                .u.us = 0},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
	{bp_usPortNum,               .u.us = 0},
	{bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6305 | BP_AFE_FE_REV_6305_REV_12_5_80 | BP_AFE_FE_RNC},
	{bp_usGpioAFELDPwr,          .u.us = BP_GPIO_87_AH}, // Line Driver 0 = "Int"
	{bp_usGpioAFELDData,         .u.us = BP_GPIO_17_AH},
	{bp_usGpioAFELDClk,          .u.us = BP_GPIO_34_AH},
	{bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_32_AH},
	{bp_usGpioAFELDRelay,        .u.us = BP_GPIO_31_AH},
	{bp_usIntfEnd},
	{bp_usIntfId,                .u.us = 1},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
	{bp_usPortNum,               .u.us = 1},
	{bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH1 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_146__REV_12_3_85 | BP_AFE_FE_RNC},
	{bp_usGpioAFELDPwr,          .u.us = BP_GPIO_33_AH}, // Line Driver 1 = "Ext"
	{bp_usGpioAFELDData,         .u.us = BP_GPIO_86_AH},
	{bp_usGpioAFELDClk,          .u.us = BP_GPIO_84_AH},
	{bp_usIntfEnd},	
	//secondary definitions for AfeIds
	{bp_usIntfId,                .u.us = 2},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
	{bp_usPortNum,               .u.us = 0},
	{bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_146__REV_12_3_60 | BP_AFE_FE_RNC},
	{bp_usIntfEnd},
	{bp_usIntfId,                .u.us = 3},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
	{bp_usPortNum,               .u.us = 1},
	{bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH1 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_146__REV_12_3_85 | BP_AFE_FE_RNC},
	{bp_usIntfEnd},
	{bp_usIntfId,                .u.us = 4},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
	{bp_usPortNum,               .u.us = 0},
	{bp_usGpioI2cSda,            .u.us = BP_GPIO_12_AH },
	{bp_usGpioI2cScl,            .u.us = BP_GPIO_3_AH },
	{bp_usIntfEnd},
	{bp_usIntfId,                .u.us = 5},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
	{bp_usPortNum,               .u.us = 1},
	{bp_usGpioI2cSda,            .u.us = BP_GPIO_25_AH },
	{bp_usGpioI2cScl,            .u.us = BP_GPIO_22_AH },
	{bp_usIntfEnd},
	/* Must keep the SGMII intf order as below. Do not shuffle or delete SGMII interface */
	{bp_usIntfId,                .u.us = 7},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
	{bp_usPortNum,               .u.us = 6},
	{bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
	{bp_usIntfMgmtBusNum,        .u.us = 1},
	{bp_usGpioSfpModDetect,      .u.us = BP_GPIO_30_AL},
	{bp_usSfpSigDetect,          .u.us = BP_GPIO_35_AL},
	{bp_usIntfEnd},
	{bp_usIntfId,                .u.us = 8},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
	{bp_usPortNum,               .u.us = 7},
	{bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
	{bp_usIntfMgmtBusNum,        .u.us = 0},
	{bp_usGpioSfpModDetect,      .u.us = BP_GPIO_16_AL},
	{bp_usSfpSigDetect,          .u.us = BP_GPIO_5_AL},
	{bp_usIntfEnd},
	{bp_usPcmSdin,               .u.us = BP_GPIO_80_AH},
	{bp_usPcmSdout,              .u.us = BP_GPIO_81_AH},
	{bp_usPcmClk,                .u.us = BP_GPIO_85_AH},
	{bp_usPcmFs,                 .u.us = BP_GPIO_82_AH},
	{bp_usI2sRxSdata,            .u.us = BP_GPIO_4_AH},
	{bp_usI2sTxLrck,             .u.us = BP_GPIO_9_AH},
	{bp_usI2sTxSclk,             .u.us = BP_GPIO_7_AH},
	{bp_usI2sSdata,              .u.us = BP_GPIO_0_AH},
	{bp_usSpiSlaveSelectNum,     .u.us = 1},
	{bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_20_AL},
	{bp_last}
};

static bp_elem_t g_bcm963146ref1d_rgmii[] = {
	{bp_cpBoardId,               .u.cp = "63146REF1D_RGM"},
	{bp_usGphyBaseAddress,       .u.us = BCM963146_PHY_BASE},
	{bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
	{bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
	{bp_ucPhyAddress,            .u.uc = 0x1e},
	{bp_ulPortMap,               .u.ul = 0xff}, //QGPHYs+SGPHY+RGMII+AE+SGMII
	// QGPHYs
	{bp_ulPhyId0,                .u.ul = (BCM963146_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
	{bp_ulPhyId1,                .u.ul = (BCM963146_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
	{bp_ulPhyId2,                .u.ul = (BCM963146_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
	{bp_ulPhyId3,                .u.ul = (BCM963146_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
	// SGPHY
	{bp_ulPhyId4,                .u.ul = (BCM963146_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
	// RGMII
	{bp_ulPhyId5,                .u.ul =  0x19 | PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},
	// Serdes
	{bp_ulPhyId6,                .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},    //SGMII
	{bp_ulPhyId7,                .u.ul = 7 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},    //AE
  
	{bp_elemTemplate,            .u.bp_elemp = g_bcm963146ref1d},  
	{bp_last}  
};
static bp_elem_t g_bcm963146pref1d_2X8[] = {
	{bp_cpBoardId,               .u.cp = "63146PREF1D_2X8"},
	{bp_usIntfId,                .u.us = 0},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
	{bp_usPortNum,               .u.us = 0},
	{bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6305 | BP_AFE_FE_REV_6305_REV_12_5_80 | BP_AFE_FE_RNC},
	{bp_usGpioAFELDPwr,          .u.us = BP_GPIO_87_AH}, // Line Driver 0 = "Int"
	{bp_usGpioAFELDData,         .u.us = BP_GPIO_17_AH},
	{bp_usGpioAFELDClk,          .u.us = BP_GPIO_34_AH},
	{bp_usGpioAFELDPwrBoost,     .u.us = BP_GPIO_32_AH},
	{bp_usGpioAFELDRelay,        .u.us = BP_GPIO_19_AH},
	{bp_usIntfEnd},
	{bp_usIntfId,                .u.us = 1},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
	{bp_usPortNum,               .u.us = 1},
	{bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH1 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_146__REV_12_3_85 | BP_AFE_FE_RNC},
	{bp_usGpioAFELDPwr,          .u.us = BP_GPIO_33_AH}, // Line Driver 1 = "Ext"
	{bp_usGpioAFELDData,         .u.us = BP_GPIO_86_AH},
	{bp_usGpioAFELDClk,          .u.us = BP_GPIO_84_AH},
	{bp_usIntfEnd},	

	{bp_elemTemplate,            .u.bp_elemp = g_bcm963146ref1d},  
	{bp_last}  
};


static bp_elem_t g_bcm963146ref2d[] = {
	{bp_cpBoardId,               .u.cp = "963146REF2D"},
	{bp_usSerialLedData,         .u.us = BP_GPIO_8_AH},
	{bp_usSerialLedClk,          .u.us = BP_GPIO_6_AH},
	{bp_usSerialLedMask,         .u.us = BP_GPIO_11_AH},

	{bp_usGphyBaseAddress,       .u.us = BCM963146_PHY_BASE},
	{bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
	{bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
	{bp_ucPhyAddress,            .u.uc = 0x1e},
	// QGPHYs+SGPHY
	{bp_ulPortMap,               .u.ul = 0xdf},
	{bp_ulPhyId0,                .u.ul = (BCM963146_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
	{bp_ulPhyId1,                .u.ul = (BCM963146_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
	{bp_ulPhyId2,                .u.ul = (BCM963146_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
	{bp_ulPhyId3,                .u.ul = (BCM963146_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
	{bp_ulPhyId4,                .u.ul = (BCM963146_PHY_BASE + 0x04) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},

	{bp_ulPhyId6,                .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
	{bp_ulPhyId7,                .u.ul = 7 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},

	{bp_usIntfId,                .u.us = 0},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
	{bp_usPortNum,               .u.us = 0},
	{bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6304 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6304_REV_12_4_80 | BP_AFE_FE_RNC },
	{bp_usGpioAFELDPwr,          .u.us = BP_GPIO_87_AH}, // Line Driver 0 = "Int"
	{bp_usGpioAFELDData,         .u.us = BP_GPIO_17_AH},
	{bp_usGpioAFELDClk,          .u.us = BP_GPIO_34_AH},
	{bp_usIntfEnd},
	{bp_usIntfId,                .u.us = 1},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
	{bp_usPortNum,               .u.us = 1},
	{bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH1 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_146__REV_12_3_85 | BP_AFE_FE_RNC },
	{bp_usGpioAFELDPwr,          .u.us = BP_GPIO_33_AH}, // Line Driver 1 = "Ext"
	{bp_usGpioAFELDData,         .u.us = BP_GPIO_86_AH},
	{bp_usGpioAFELDClk,          .u.us = BP_GPIO_84_AH},
	{bp_usIntfEnd},	
	//secondary definitions for AfeIds
	{bp_usIntfId,                .u.us = 2},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
	{bp_usPortNum,               .u.us = 0},
	{bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH0 | BP_AFE_LD_6304 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6304_REV_12_4_80 | BP_AFE_FE_RNC },
	{bp_usIntfEnd},
	{bp_usIntfId,                .u.us = 3},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_xDSL},
	{bp_usPortNum,               .u.us = 1},
	{bp_ulAfeId,                 .u.ul = BP_AFE_CHIP_GFAST_CH1 | BP_AFE_LD_6303 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6303_146__REV_12_3_85 | BP_AFE_FE_RNC },
	{bp_usIntfEnd},
	{bp_usIntfId,                .u.us = 4},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
	{bp_usPortNum,               .u.us = 0},
	{bp_usGpioI2cSda,            .u.us = BP_GPIO_12_AH },
	{bp_usGpioI2cScl,            .u.us = BP_GPIO_3_AH },
	{bp_usIntfEnd},
	{bp_usIntfId,                .u.us = 5},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
	{bp_usPortNum,               .u.us = 1},
	{bp_usGpioI2cSda,            .u.us = BP_GPIO_25_AH },
	{bp_usGpioI2cScl,            .u.us = BP_GPIO_22_AH },
	{bp_usIntfEnd},
	/* Must keep the SGMII intf order as below. Do not shuffle or delete SGMII interface */
	{bp_usIntfId,                .u.us = 6},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
	{bp_usPortNum,               .u.us = 6},
	{bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
	{bp_usIntfMgmtBusNum,        .u.us = 1},
	{bp_usGpioSfpModDetect,      .u.us = BP_GPIO_30_AL},
	{bp_usSfpSigDetect,          .u.us = BP_GPIO_35_AL},
	{bp_usIntfEnd},
	{bp_usIntfId,                .u.us = 7},
	{bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
	{bp_usPortNum,               .u.us = 7},
	{bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
	{bp_usIntfMgmtBusNum,        .u.us = 0},
	{bp_usGpioSfpModDetect,      .u.us = BP_GPIO_16_AL},
	{bp_usSfpSigDetect,          .u.us = BP_GPIO_5_AL},
	{bp_usIntfEnd},

	{bp_usI2sTxSclk,             .u.us = BP_GPIO_57_AH},
	{bp_usI2sTxLrck,             .u.us = BP_GPIO_58_AH},
	{bp_usI2sRxSdata,            .u.us = BP_GPIO_59_AH},
	{bp_usI2sTxSdata,            .u.us = BP_GPIO_60_AH},
	{bp_usI2sTxMclk,             .u.us = BP_GPIO_61_AH},
	{bp_usI2sRxSclk,             .u.us = BP_GPIO_62_AH},
	{bp_usI2sRxLrck,             .u.us = BP_GPIO_63_AH},
	{bp_usI2sRxMclk,             .u.us = BP_GPIO_64_AH},

	{bp_usPcmSdin,               .u.us = BP_GPIO_80_AH},
	{bp_usPcmSdout,              .u.us = BP_GPIO_81_AH},
	{bp_usPcmClk,                .u.us = BP_GPIO_85_AH},
	{bp_usPcmFs,                 .u.us = BP_GPIO_82_AH},
	{bp_usSpiSlaveSelectNum,     .u.us = 1},
	{bp_usSpiSlaveSelectGpioNum, .u.us = BP_GPIO_20_AL},
	{bp_last}
};

bp_elem_t * g_BoardParms[] = {g_bcm963146sv_ddr4, g_bcm963146sv_ddr4_rgmii, g_bcm963146sv, g_bcm963146sv_rgmii, g_bcm963146ref1d, g_bcm963146pref1d_2X8, g_bcm963146ref1d_rgmii, g_bcm963146ref2d, 0};
