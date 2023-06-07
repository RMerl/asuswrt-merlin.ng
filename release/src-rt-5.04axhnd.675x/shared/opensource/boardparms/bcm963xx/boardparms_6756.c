#include "bp_defs.h"
#include "boardparms.h"
#include "bcmSpiRes.h"

#define MOVED_DT(x...)

#define BCM96756_PHY_BASE             0x8

static bp_elem_t g_bcm96756sv[] = {
    {bp_cpBoardId,               .u.cp = "96756SV"},

    MOVED_DT({bp_usGphyBaseAddress,       .u.us = BCM96756_PHY_BASE},) 
    MOVED_DT({bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},)
    MOVED_DT({bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},)
    MOVED_DT({bp_ulPortMap,               .u.ul = 0x23},)
    MOVED_DT({bp_ulPhyId0,                .u.ul = (BCM96756_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},)
    MOVED_DT({bp_ulPhyId1,                .u.ul = 0x19 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},)
    MOVED_DT({bp_ulPhyId5,                .u.ul = 6 | PHY_INTEGRATED_VALID | MAC_IF_SERDES},)

    {bp_usIntfId,                .u.us = 0},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
    {bp_usPortNum,               .u.us = 0},
    {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
    {bp_usIntfMgmtBusNum,        .u.us = 0},
    {bp_usGpioSfpModDetect,      .u.us = BP_GPIO_9_AL},
    {bp_usSfpSigDetect,          .u.us = BP_GPIO_26_AL },
    {bp_usIntfEnd},

    {bp_usIntfId,                .u.us = 1},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
    {bp_usPortNum,               .u.us = 0},
    {bp_usIntfEnd},

    {bp_last}
};

static bp_elem_t g96756ref1[] = {
    {bp_cpBoardId,               .u.cp = "96756REF1"},

    MOVED_DT({bp_usGphyBaseAddress,       .u.us = BCM96756_PHY_BASE},)
    MOVED_DT({bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},)
    MOVED_DT({bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},)
    MOVED_DT({bp_ulPortMap,               .u.ul = 0x03},)
    MOVED_DT({bp_ulPhyId0,                .u.ul = (BCM96756_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},)
    MOVED_DT({bp_ulPhyId1,                .u.ul = RGMII_DIRECT_3P3V | EXTSW_CONNECTED},)
    MOVED_DT({bp_ulPortFlags,             .u.ul = PORT_FLAG_TX_INTERNAL_DELAY | PORT_FLAG_RX_INTERNAL_DELAY},)

    MOVED_DT({bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},)
    MOVED_DT({bp_ucPhyAddress,            .u.uc = 0x0},)
    MOVED_DT({bp_usConfigType,            .u.us = BP_ENET_CONFIG_MDIO},)
    MOVED_DT({bp_ulPortMap,               .u.ul = 0x0f},)
    MOVED_DT({bp_ulPhyId0,                .u.ul = BP_PHY_ID_0 | CONNECTED_TO_EXTERN_SW},)
    MOVED_DT({bp_ulPhyId1,                .u.ul = BP_PHY_ID_1 | CONNECTED_TO_EXTERN_SW},)
    MOVED_DT({bp_ulPhyId2,                .u.ul = BP_PHY_ID_2 | CONNECTED_TO_EXTERN_SW},)
    MOVED_DT({bp_ulPhyId3,                .u.ul = BP_PHY_ID_3 | CONNECTED_TO_EXTERN_SW},)

    {bp_last}
};

static bp_elem_t g96756ref1_sg[] = {
    {bp_cpBoardId,               .u.cp = "96756REF1_SG"},

    MOVED_DT({bp_usGphyBaseAddress,       .u.us = BCM96756_PHY_BASE},)
    MOVED_DT({bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},)
    MOVED_DT({bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},)
    MOVED_DT({bp_ulPortMap,               .u.ul = 0x21},)
    MOVED_DT({bp_ulPhyId0,                .u.ul = (BCM96756_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},)
    MOVED_DT({bp_ulPhyId5,                .u.ul = 6 |  PHY_INTEGRATED_VALID | MAC_IF_SERDES | EXTSW_CONNECTED},)

    MOVED_DT({bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},)
    MOVED_DT({bp_ucPhyAddress,            .u.uc = 0x0},)
    MOVED_DT({bp_usConfigType,            .u.us = BP_ENET_CONFIG_MDIO},)
    MOVED_DT({bp_ulPortMap,               .u.ul = 0x0f},)
    MOVED_DT({bp_ulPhyId0,                .u.ul = BP_PHY_ID_0 | CONNECTED_TO_EXTERN_SW},)
    MOVED_DT({bp_ulPhyId1,                .u.ul = BP_PHY_ID_1 | CONNECTED_TO_EXTERN_SW},)
    MOVED_DT({bp_ulPhyId2,                .u.ul = BP_PHY_ID_2 | CONNECTED_TO_EXTERN_SW},)
    MOVED_DT({bp_ulPhyId3,                .u.ul = BP_PHY_ID_3 | CONNECTED_TO_EXTERN_SW},)

    {bp_last}
};

static bp_elem_t g_bcm96756rfdvt[] = {
    {bp_cpBoardId,               .u.cp = "96756RF_DVT"},

    MOVED_DT({bp_usGphyBaseAddress,       .u.us = BCM96756_PHY_BASE},) 
    MOVED_DT({bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},)
    MOVED_DT({bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},)
    MOVED_DT({bp_ulPortMap,               .u.ul = 0x21},)
    MOVED_DT({bp_ulPhyId0,                .u.ul = (BCM96756_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},)
    MOVED_DT({bp_ulPhyId5,                .u.ul = 6 | PHY_INTEGRATED_VALID | MAC_IF_SERDES},)
    MOVED_DT({bp_ulPhyId5,                .u.ul = 0x1e | PHY_EXTERNAL | PHY_TYPE_CL45GPHY},)

    {bp_last}
};

static bp_elem_t g_bcm947623eap6l[] = {
    {bp_cpBoardId,               .u.cp = "947623EAP6L"},

    MOVED_DT({bp_usGphyBaseAddress,       .u.us = BCM96756_PHY_BASE},)
    MOVED_DT({bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},)
    MOVED_DT({bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},)
    MOVED_DT({bp_ulPortMap,               .u.ul = 0x21},)
    MOVED_DT({bp_ulPhyId0,                .u.ul = (BCM96756_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},)
    MOVED_DT({bp_ulPhyId5,                .u.ul = 6 | PHY_INTEGRATED_VALID | MAC_IF_SERDES},)
    MOVED_DT({bp_ulPhyId5,                .u.ul = 0x1f | PHY_EXTERNAL | PHY_TYPE_CL45GPHY},)

    {bp_last}
};

static bp_elem_t g_bcm947623eap6ll[] = {
    {bp_cpBoardId,               .u.cp = "947623EAP6LL"},

    MOVED_DT({bp_usGphyBaseAddress,       .u.us = BCM96756_PHY_BASE},)
    MOVED_DT({bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},)
    MOVED_DT({bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},)
    MOVED_DT({bp_ulPortMap,               .u.ul = 0x21},)
    MOVED_DT({bp_ulPhyId0,                .u.ul = (BCM96756_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},)
    MOVED_DT({bp_ulPhyId5,                .u.ul = 6 | PHY_INTEGRATED_VALID | MAC_IF_SERDES},)
    MOVED_DT({bp_ulPhyId5,                .u.ul = 0x1f | PHY_EXTERNAL | PHY_TYPE_CL45GPHY},)

    {bp_last}
};

static bp_elem_t g_bcm947623xeapl2526[] = {
    {bp_cpBoardId,               .u.cp = "947623XEAPL2526"},
    {bp_last}
};

static bp_elem_t g_bcm947623eapl2526[] = {
    {bp_cpBoardId,               .u.cp = "947623EAPL2526"},
    {bp_last}
};

static bp_elem_t g_bcm96757sv[] = {
    {bp_cpBoardId,               .u.cp = "96757SV"},

    MOVED_DT({bp_usGphyBaseAddress,       .u.us = BCM96756_PHY_BASE},)
    MOVED_DT({bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},)
    MOVED_DT({bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},)
    MOVED_DT({bp_ulPortMap,               .u.ul = 0x61},)
    MOVED_DT({bp_ulPhyId0,                .u.ul = (BCM96756_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},)
    MOVED_DT({bp_ulPhyId5,                .u.ul = 6 | PHY_INTEGRATED_VALID | MAC_IF_SERDES},)
    MOVED_DT({bp_ulPhyId6,                .u.ul = 7 | PHY_INTEGRATED_VALID | MAC_IF_SERDES},)

    {bp_usIntfId,                .u.us = 0},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
    {bp_usPortNum,               .u.us = 0},
    {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
    {bp_usIntfMgmtBusNum,        .u.us = 0},
    {bp_usGpioSfpModDetect,      .u.us = BP_GPIO_9_AL},
    {bp_usSfpSigDetect,          .u.us = BP_GPIO_26_AL },
    {bp_usIntfEnd},

    {bp_usIntfId,                .u.us = 1},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
    {bp_usPortNum,               .u.us = 1},
    {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
    {bp_usIntfMgmtBusNum,        .u.us = 1},
    {bp_usGpioSfpModDetect,      .u.us = BP_GPIO_66_AL},
    {bp_usSfpSigDetect,          .u.us = BP_GPIO_67_AL },
    {bp_usIntfEnd},

    {bp_usIntfId,                .u.us = 2},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
    {bp_usPortNum,               .u.us = 0},
    {bp_usIntfEnd},

    {bp_usIntfId,                .u.us = 3},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
    {bp_usPortNum,               .u.us = 1},
    {bp_usIntfEnd},
    
    {bp_last}
};

static bp_elem_t g_bcm96757ref1t[] = {
    {bp_cpBoardId,               .u.cp = "96757REF1T"},

    MOVED_DT({bp_usGphyBaseAddress,       .u.us = BCM96756_PHY_BASE},)
    MOVED_DT({bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},)
    MOVED_DT({bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},)
    MOVED_DT({bp_ulPortMap,               .u.ul = 0x61},)
    MOVED_DT({bp_ulPhyId0,                .u.ul = (BCM96756_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},)
    MOVED_DT({bp_ulPhyId5,                .u.ul = 6 | PHY_INTEGRATED_VALID | MAC_IF_SERDES},)
    MOVED_DT({bp_ulPhyId5,                .u.ul = 0x1f | PHY_EXTERNAL | PHY_TYPE_CL45GPHY},)
    MOVED_DT({bp_ulPhyId6,                .u.ul = 7 | PHY_INTEGRATED_VALID | MAC_IF_SERDES},)
    MOVED_DT({bp_ulPhyId6,                .u.ul = 0x1e | PHY_EXTERNAL | PHY_TYPE_CL45GPHY},)

    {bp_last}
};

static bp_elem_t g_bcm96757ref2t[] = {
    {bp_cpBoardId,               .u.cp = "96757REF2T"},

    MOVED_DT({bp_usGphyBaseAddress,       .u.us = BCM96756_PHY_BASE},)
    MOVED_DT({bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},)
    MOVED_DT({bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},)
    MOVED_DT({bp_ulPortMap,               .u.ul = 0x61},)
    MOVED_DT({bp_ulPhyId0,                .u.ul = (BCM96756_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},)
    MOVED_DT({bp_ulPhyId5,                .u.ul = 6 | PHY_INTEGRATED_VALID | MAC_IF_SERDES},)
    MOVED_DT({bp_ulPhyId5,                .u.ul = 0x1f | PHY_EXTERNAL | PHY_TYPE_CL45GPHY},)
    MOVED_DT({bp_ulPhyId6,                .u.ul = 7 | PHY_INTEGRATED_VALID | MAC_IF_SERDES},)
    MOVED_DT({bp_ulPhyId6,                .u.ul = 0x1e | PHY_EXTERNAL | PHY_TYPE_CL45GPHY},)

    {bp_last}
};

static bp_elem_t g_bcm96756rfdvt_fake[] = {
    {bp_cpBoardId,               .u.cp = "96756RFDVT_FAKE"},

    MOVED_DT({bp_usGphyBaseAddress,       .u.us = BCM96756_PHY_BASE},) 
    MOVED_DT({bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},)
    MOVED_DT({bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},)
    MOVED_DT({bp_ulPortMap,               .u.ul = 0x3},)
    MOVED_DT({bp_ulPhyId0,                .u.ul = (BCM96756_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},)
    MOVED_DT({bp_ulPhyId1,                .u.ul = 0x19 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},)
    {bp_last}
};

/* copy from g_bcm96756sv[] */
static bp_elem_t rt_ax55[] = {
    {bp_cpBoardId,               .u.cp = "RT-AX55"},

    MOVED_DT({bp_usGphyBaseAddress,       .u.us = BCM96756_PHY_BASE},) 
    MOVED_DT({bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},)
    MOVED_DT({bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},)
    MOVED_DT({bp_ulPortMap,               .u.ul = 0x23},)
    MOVED_DT({bp_ulPhyId0,                .u.ul = (BCM96756_PHY_BASE + 0x00) | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},)
    MOVED_DT({bp_ulPhyId1,                .u.ul = 0x19 |  PHY_INTEGRATED_VALID | MAC_IF_RGMII_1P8V | PHY_EXTERNAL},)
    MOVED_DT({bp_ulPhyId5,                .u.ul = 6 | PHY_INTEGRATED_VALID | MAC_IF_SERDES},)

    {bp_usIntfId,                .u.us = 0},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_SGMII},
    {bp_usPortNum,               .u.us = 0},
    {bp_usIntfMgmtType,          .u.us = BP_INTF_MGMT_TYPE_I2C},
    {bp_usIntfMgmtBusNum,        .u.us = 0},
    {bp_usGpioSfpModDetect,      .u.us = BP_GPIO_9_AL},
    {bp_usSfpSigDetect,          .u.us = BP_GPIO_26_AL },
    {bp_usIntfEnd},

    {bp_usIntfId,                .u.us = 1},
    {bp_usIntfType,              .u.us = BP_INTF_TYPE_I2C},
    {bp_usPortNum,               .u.us = 0},
    {bp_usIntfEnd},

    {bp_last}
};

static bp_elem_t xd4pro[] = {
    {bp_cpBoardId,               .u.cp = "XD4PRO"},

    {bp_last}
};

static bp_elem_t xc5[] = {
    {bp_cpBoardId,               .u.cp = "XC5"},

    {bp_last}
};

static bp_elem_t RT_AX58U_V2[] = {
    {bp_cpBoardId,               .u.cp = "RT_AX58U_V2"},

    {bp_last}
};

static bp_elem_t xt8pro[] = {
    {bp_cpBoardId,               .u.cp = "XT8PRO"},

    {bp_last}
};

static bp_elem_t et8pro[] = {
    {bp_cpBoardId,               .u.cp = "ET8PRO"},

    {bp_last}
};

static bp_elem_t et8_v2[] = {
    {bp_cpBoardId,               .u.cp = "ET8_V2"},

    {bp_last}
};

static bp_elem_t RP_AX58[] = {
    {bp_cpBoardId,               .u.cp = "RP_AX58"},

    {bp_last}
};

static bp_elem_t xt8pro_gpy211[] = {
    {bp_cpBoardId,               .u.cp = "XT8PRO_GPY211"},

    {bp_last}
};

static bp_elem_t bm68[] = {
    {bp_cpBoardId,               .u.cp = "BM68"},

    {bp_last}
};

static bp_elem_t xt8_v2[] = {
    {bp_cpBoardId,               .u.cp = "XT8_V2"},

    {bp_last}
};

static bp_elem_t et8pro_gpy211[] = {
    {bp_cpBoardId,               .u.cp = "ET8PRO_GPY211"},

    {bp_last}
};

static bp_elem_t xd4proipa[] = {
    {bp_cpBoardId,               .u.cp = "XD4PROIPA"},

    {bp_last}
};

static bp_elem_t TUF_AX3000_V2[] = {
    {bp_cpBoardId,               .u.cp = "TUF_AX3000_V2"},

    {bp_last}
};

static bp_elem_t RT_AXE7800[] = {
    {bp_cpBoardId,               .u.cp = "RT_AXE7800"},

    {bp_last}
};

static bp_elem_t RT_AX3000N[] = {
    {bp_cpBoardId,               .u.cp = "RT_AX3000N"},

    {bp_last}
};

static bp_elem_t br63[] = {
    {bp_cpBoardId,               .u.cp = "BR63"},

    {bp_last}
};

static bp_elem_t xd4proipa_ddr3[] = {
    {bp_cpBoardId,               .u.cp = "XD4PROIPA_DDR3"},

    {bp_last}
};

static bp_elem_t xt8_v2_50991[] = {
    {bp_cpBoardId,               .u.cp = "XT8_V2_50991"},

    {bp_last}
};

static bp_elem_t eba63[] = {
    {bp_cpBoardId,               .u.cp = "EBA63"},

    {bp_last}
};

bp_elem_t * g_BoardParms[] = {g_bcm96756sv, /*g96756ref1, g96756ref1_sg, g_bcm96756rfdvt, g_bcm947623eap6l, g_bcm947623eap6ll,
                              g_bcm96757sv, g_bcm96757ref1t, g_bcm96757ref2t, g_bcm96756rfdvt_fake, rt_ax55, */xd4pro, RT_AX58U_V2, xt8pro, et8pro, RP_AX58, xt8pro_gpy211, et8pro_gpy211, xd4proipa, TUF_AX3000_V2, RT_AXE7800, xt8_v2, RT_AX3000N, xd4proipa_ddr3, et8_v2, bm68, br63, xt8_v2_50991, xc5, eba63, 0};
