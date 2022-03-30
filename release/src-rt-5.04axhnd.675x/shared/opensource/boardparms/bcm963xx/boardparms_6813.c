#include "bp_defs.h"
#include "boardparms.h"
#include "bcmSpiRes.h"

#define BCM96813_PHY_BASE             0x1

static bp_elem_t g_bcm96813ref1[] = {
  {bp_cpBoardId,               .u.cp = "96813REF1"},
  {bp_last}
};

static bp_elem_t g_bcm96813ref3[] = {
  {bp_cpBoardId,               .u.cp = "96813REF3"},
  {bp_usGphyBaseAddress,       .u.us = BCM96813_PHY_BASE},
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY}, // Runner
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ucPhyAddress,            .u.uc = 0x1e},
  {bp_ulPortMap,               .u.ul = 0xef},   // QGPHYs + 10GPHY + Serdes
  {bp_ulPhyId0,                .u.ul = (BCM96813_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId1,                .u.ul = (BCM96813_PHY_BASE + 0x01) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId2,                .u.ul = (BCM96813_PHY_BASE + 0x02) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
  {bp_ulPhyId3,                .u.ul = (BCM96813_PHY_BASE + 0x03) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},

  {bp_ulPhyId5,                .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulPhyId5,                .u.ul = 0x11 | PHY_EXTERNAL | PHY_TYPE_CL45GPHY},
  {bp_ulPhyId6,                .u.ul = 7 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},
  {bp_ulPhyId7,                .u.ul = 8 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES},

  {bp_usIntfId,                .u.us = 0},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
  {bp_usPortNum,               .u.us = 0},
  {bp_usGpioI2cSda,            .u.us = BP_GPIO_13_AH },
  {bp_usGpioI2cScl,            .u.us = BP_GPIO_12_AH },
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 1},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
  {bp_usPortNum,               .u.us = 1},
  {bp_usGpioI2cSda,            .u.us = BP_GPIO_15_AH },
  {bp_usGpioI2cScl,            .u.us = BP_GPIO_14_AH },
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 2},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
  {bp_usPortNum,               .u.us = 5},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 3},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
  {bp_usPortNum,               .u.us = 6},
  {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
  {bp_usIntfMgmtBusNum,        .u.us = 0},
  {bp_usGpioSfpModDetect,      .u.us = BP_GPIO_4_AL},
  {bp_usSfpSigDetect,          .u.us = BP_GPIO_1_AL},
  {bp_usIntfEnd},
  {bp_usIntfId,                .u.us = 4},
  {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
  {bp_usPortNum,               .u.us = 7},
  {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
  {bp_usIntfMgmtBusNum,        .u.us = 1},
  {bp_usGpioSfpModDetect,      .u.us = BP_GPIO_5_AL},
  {bp_usSfpSigDetect,          .u.us = BP_GPIO_2_AL},
  {bp_usIntfEnd},

  {bp_usMiiMdc,                .u.us = BP_GPIO_54_AH},
  {bp_usMiiMdio,               .u.us = BP_GPIO_55_AH},
  {bp_ucDspType0,              .u.uc = BP_VOIP_DSP},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_last}
};

bp_elem_t * g_BoardParms[] = {g_bcm96813ref1, g_bcm96813ref3, 0};
